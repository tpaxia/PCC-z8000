#!/usr/bin/env python3
"""Fresh, isolated compile/assemble/link/execute tests for the K&R Z8002 port."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

HERE = Path(__file__).resolve().parent
TEST = HERE.parent
TARGET = TEST.parent


def command(argv, log, *, source=None, output=None, cwd=None):
    try:
        result = subprocess.run(
            [str(a) for a in argv], input=source, capture_output=True, timeout=20,
            cwd=cwd
        )
    except subprocess.TimeoutExpired:
        log.write_text("timeout after 20 seconds\n")
        return False, "timeout after 20 seconds"
    stdout = result.stdout.decode(errors="replace")
    stderr = result.stderr.decode(errors="replace")
    log.write_text(stdout + stderr)
    if output is not None:
        output.write_bytes(result.stdout)
    return result.returncode == 0, stderr if output is not None else stdout + stderr


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", action="store_true", help="expected failures also fail")
    parser.add_argument("--case", action="append", help="run named cases only")
    parser.add_argument("--build-dir", type=Path, default=HERE / "build")
    parser.add_argument("--compact", action="store_true", help="test the native oz8 assembly optimizer")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    build.mkdir(parents=True, exist_ok=True)
    expected = json.loads((HERE / "expectations.json").read_text())
    diagnostics = json.loads((HERE / "diagnostics.json").read_text())
    codegen = json.loads((HERE / "codegen.json").read_text())
    cases = [(p.stem, p, 0) for p in sorted(HERE.glob("*.c"))]
    cases += [(p.stem, p, 0) for p in sorted(HERE.glob("*.az8"))]
    # Existing tests go through the same fresh pipeline, without using old .bout files.
    cases += [(p.stem, p, {"hello": 42, "arith": 120}.get(p.stem, 0))
              for p in sorted(TEST.glob("*.c"))]
    cases += [("reject_" + p.stem, p, None)
              for p in sorted((HERE / "diagnostics").glob("*.c"))]
    if args.case:
        names = {name for name, _, _ in cases}
        unknown = set(args.case) - names
        if unknown:
            parser.error("unknown cases: " + ", ".join(sorted(unknown)))
        cases = [case for case in cases if case[0] in args.case]

    setup = [
        ["make", "-C", TARGET / "cz8"],
        ["make", "-C", TARGET / "az8"],
        ["make", "-C", TEST, "run_emu"],
        ["make", "-C", TEST, "../oz8"],
        [sys.executable, HERE / "check_softfp.py"],
        ["cc", "-O", "-w", "-Wno-implicit-int",
         "-Wno-implicit-function-declaration", "-Wno-int-conversion",
         "-Wno-return-mismatch", "-o", build / "ldz8", TARGET / "ldz8.c"],
    ]
    for i, argv in enumerate(setup):
        ok, detail = command(argv, build / ("setup-%d.log" % i))
        if not ok:
            print("Setup failed:\n" + detail)
            return 1
    cz8 = TARGET / "cz8" / "cz8"
    az8 = TARGET / "az8" / "az8"
    shared = []
    runtime = [("crt0", TARGET / "crt0.az8"), ("exit", TARGET / "lib" / "exit.az8")]
    # Every case links the complete integer and double-addition runtime.
    if (TARGET / "lib" / "arith.az8").exists():
        runtime.append(("liblong", TARGET / "lib" / "arith.az8"))
    runtime.append(("libfloat", TARGET / "lib" / "float.az8"))
    soft = build / "softfp.az8"
    ok, detail = command([cz8], build / "softfp-compile.log",
                         source=(TARGET / "lib" / "softfp.c").read_bytes(), output=soft)
    if not ok:
        print("Floating runtime compilation failed:\n" + detail)
        return 1
    if args.compact:
        optimized = subprocess.run([str(TARGET / "oz8")],
                                   input=soft.read_bytes(), capture_output=True, check=True)
        soft.write_bytes(optimized.stdout)
    runtime.append(("csv", TARGET / "lib" / "csv.az8"))
    runtime.append(("softfp", soft))
    for name, source in runtime:
        obj = build / (name + ".b")
        local = build / (name + ".az8")
        local.write_bytes(source.read_bytes())
        ok, detail = command([az8, "-o", obj.name, local.name],
                             build / (name + ".log"), cwd=build)
        if not ok:
            print("Runtime assembly failed:\n" + detail)
            return 1
        shared.append(obj)

    counts = dict(PASS=0, XFAIL=0, XPASS=0, FAIL=0)
    for name, source, value in cases:
        directory = build / name
        directory.mkdir(exist_ok=True)
        for filename in ("test.az8", "test.b", "test.bout", "compile.log",
                         "assemble.log", "link.log", "run.log"):
            (directory / filename).unlink(missing_ok=True)
        assembly = directory / "test.az8"
        obj = directory / "test.b"
        binary = directory / "test.bout"
        stages = [
            ("compile", [cz8], source.read_bytes(), assembly),
            ("assemble", [az8, "-o", obj.name, assembly.name], None, None),
            ("link", [build / "ldz8", "-x", shared[0], "-R", "8", obj,
                      *shared[1:], "-o", binary], None, None),
            ("run", [TEST / "run_emu", binary, "-e", str(value),
                      "-c", "10000000" if name in ("float_vectors", "float_ops_vectors", "float_general", "float_convert_vectors") else "1000000"], None, None),
        ]
        if source.suffix == ".az8":
            assembly.write_bytes(source.read_bytes())
            stages = stages[1:]
        failure = None
        for stage, argv, stdin, output in stages:
            ok, detail = command(argv, directory / (stage + ".log"),
                                 source=stdin, output=output, cwd=directory)
            # This linker currently reports unresolved symbols but exits zero.
            # Do not execute the resulting incomplete image.
            if stage == "link" and "ldz8: Undefined -" in detail:
                ok = False
            if value is None:
                diagnostic = diagnostics[source.stem]
                if ok:
                    failure = ("diagnostic", "invalid program accepted")
                elif diagnostic not in detail:
                    failure = ("diagnostic", "wrong diagnostic:\n" + detail)
                break
            if stage == "compile" and ok and name in codegen and \
                    not re.search(codegen[name], assembly.read_text()):
                failure = ("codegen", "unexpected assembly encoding")
                break
            if stage == "compile" and ok and args.compact:
                optimized = subprocess.run([str(TARGET / "oz8")],
                                           input=assembly.read_bytes(), capture_output=True, check=True)
                assembly.write_bytes(optimized.stdout)
            if not ok:
                failure = (stage, detail)
                break
        expectation = expected.get(name)
        if failure is None:
            status = "XPASS" if expectation else "PASS"
            detail = ""
        elif expectation and failure[0] == expectation["stage"] and \
                expectation["contains"] in failure[1]:
            status = "XFAIL"
            detail = " (%s: %s)" % (failure[0], expectation["bug"])
        else:
            status = "FAIL"
            detail = " (%s)\n%s" % failure
        counts[status] += 1
        print(status + " " + name + detail)
    print("\n" + ", ".join("%d %s" % (v, k) for k, v in counts.items()))
    print("Assembly and stage logs: " + str(build))
    return int(bool(counts["FAIL"] or counts["XPASS"] or
                    (args.strict and counts["XFAIL"])))


if __name__ == "__main__":
    sys.exit(main())
