#!/usr/bin/env python3
"""Run the selected upstream tests as K&R C on the Z8002 emulator.

Every failure is a FAIL and makes this command exit nonzero. The manifest
records provenance, adaptations and language/target exclusions, not XFAILs.
"""
import argparse
import hashlib
import importlib.util
import json
import re
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
TEST = HERE.parent
TARGET = TEST.parent
sys.path.insert(0, str(TEST))
from toolchain import AS, LD, setup_commands
REGRESS = TEST / "regress"
spec = importlib.util.spec_from_file_location("regress", REGRESS / "run.py")
regress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(regress)

# Fail immediately with a value distinct from successful exit(0). No printf
# stubs: output-only upstream tests are adapted to check their result instead.
SUPPORT = {
"abort": b"abort() { exit(99); }\n",
"strcmp": b"""strcmp(a,b) char *a,*b;
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
""",
"strcpy": b"""char *strcpy(d,s) char *d,*s;
{
    char *p;
    p = d;
    while ((*d++ = *s++) != 0) ;
    return p;
}
""",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", help="run a manifest case by name")
    parser.add_argument("--list", action="store_true", help="list selected cases")
    parser.add_argument("--compare68k", action="store_true",
                        help="also compile with original 68000 backend; no 68000 execution")
    parser.add_argument("--build-dir", type=Path, default=HERE / "build")
    args = parser.parse_args()
    manifest = json.loads((HERE / "manifest.json").read_text())
    cases = manifest["cases"]
    if args.case:
        unknown = set(args.case) - {c["name"] for c in cases}
        if unknown:
            parser.error("unknown cases: " + ", ".join(sorted(unknown)))
        cases = [c for c in cases if c["name"] in args.case]
    if args.list:
        for case in cases:
            print(case["name"])
        return 0

    # Refuse a silent change to either the adapted source or upstream source.
    for case in cases:
        adapted = HERE / case["source"]
        original = (HERE / "gcc/original" / Path(case["upstream"]).name
                    if case["name"].startswith("gcc-")
                    else TARGET.parent / "pcc-tests" / case["upstream"])
        for path, key in [(adapted, "source_sha256"), (original, "original_sha256")]:
            if hashlib.sha256(path.read_bytes()).hexdigest() != case[key]:
                parser.error("source changed; review adaptation and update manifest: " + str(path))

    build = args.build_dir.resolve()
    build.mkdir(parents=True, exist_ok=True)
    shared = build / "runtime"
    # Reuse the established fresh tool/runtime build and validate one smoke test.
    setup = subprocess.run([sys.executable, REGRESS / "run.py", "--case", "precedence",
                            "--build-dir", str(shared)], capture_output=True, text=True)
    (build / "setup.log").write_text(setup.stdout + setup.stderr)
    if setup.returncode:
        print("Setup failed:\n" + setup.stdout + setup.stderr)
        return 1

    # The existing regression exit stub preserves R0 from main. Upstream tests
    # also call exit(status) directly, so use a real stack-argument exit here.
    exit_source = build / "exit.az8"
    exit_obj = build / "exit.b"
    exit_source.write_text("\t.text\n\t.globl\t_exit\n_exit:\n\tld\tr0,2(sp)\n\thalt\n")
    ok, detail = regress.command([AS, "-c", "-o", exit_obj.name, exit_source.name],
                                 build / "exit-assemble.log", cwd=build)
    if not ok:
        print("Exit stub assembly failed:\n" + detail)
        return 1

    m68k = None
    if args.compare68k:
        spec = importlib.util.spec_from_file_location("compare68k", TEST / "compare68k/run.py")
        compare = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(compare)
        m68k = compare.build_m68k(build)

    results = {}
    for case in cases:
        name = case["name"]
        directory = build / name
        directory.mkdir(exist_ok=True)
        for filename in ("input.c", "test.i", "test.az8", "test.b", "test.sout",
                         "preprocess.log", "compile.log", "assemble.log", "link.log", "run.log"):
            (directory / filename).unlink(missing_ok=True)
        source = directory / "input.c"
        # Include only the helpers this test actually needs; avoid changing
        # register pressure or code size with unused helper bodies.
        body = (HERE / case["source"]).read_bytes()
        helpers = b"".join(code for name, code in SUPPORT.items()
                           if re.search(rb"\b" + name.encode() + rb"\s*\(", body))
        source.write_bytes(helpers + body)
        preprocessed = directory / "test.i"
        assembly = directory / "test.az8"
        obj = directory / "test.b"
        binary = directory / "test.sout"
        stages = [
            ("preprocess", ["cc", "-E", "-P", "-undef", "-nostdinc", "-x", "c", source],
             None, preprocessed),
            ("compile", [TARGET / "cz8/cz8"], preprocessed, assembly),
            ("assemble", [AS, "-c", "-o", obj.name, assembly.name], None, None),
            ("link", [shared / "ldz8", "-x", shared / "crt0.b",  obj,
                      exit_obj, shared / "liblong.b", shared / "csv.b", shared / "libfloat.b",
                      shared / "softfp.b", "-o", binary], None, None),
            ("run", [TEST / "run_emu", binary, "-e", "0", "-c", "2000000"], None, None),
        ]
        for stage, argv, stdin, output in stages:
            ok, detail = regress.command(argv, directory / (stage + ".log"),
                                         source=stdin.read_bytes() if stdin else None,
                                         output=output, cwd=directory)
            if not ok:
                results[name] = dict(status="FAIL", stage=stage, detail=detail.strip())
                break
        else:
            results[name] = dict(status="PASS")
        result = results[name]
        if m68k and preprocessed.exists() and (directory / "compile.log").exists():
            ok, detail = regress.command([m68k], directory / "m68k.log",
                                         source=preprocessed.read_bytes(),
                                         output=directory / "m68k.s", cwd=directory)
            result["m68k_compile"] = "PASS" if ok else "FAIL"
            if not ok:
                result["m68k_detail"] = detail.strip()
        print(result["status"], name, result.get("stage", ""))
        if result["status"] == "FAIL":
            print(result["detail"])

    (build / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    failed = sum(r["status"] == "FAIL" for r in results.values())
    if m68k:
        compiled = sum(r.get("m68k_compile") == "PASS" for r in results.values())
        print(f"68000 backend: {compiled} of {len(results)} compiled; no 68000 execution")
    print(f"{len(results)-failed} PASS, {failed} FAIL; logs in {build}")
    return int(bool(failed))


if __name__ == "__main__":
    sys.exit(main())
