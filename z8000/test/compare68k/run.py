#!/usr/bin/env python3
"""Record full-pipeline results for 68000-to-Z8000 parity probes.

This is a discovery suite: failures are reported, never hidden as XFAIL.
It is separate from the passing regression suite until fixes are implemented.
"""
import importlib.util
from pathlib import Path
import subprocess
import sys
import json
import shutil

HERE = Path(__file__).resolve().parent
REGRESS = HERE.parent / "regress"
TARGET = HERE.parent.parent
spec = importlib.util.spec_from_file_location("regress", REGRESS / "run.py")
regress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(regress)


def build_m68k(build):
    """Build original backend with host compatibility edits only."""
    directory = build / "m68k-host"
    directory.mkdir(exist_ok=True)
    original = TARGET.parent / "68000" / "c68"
    names = ["cgram", "xdefs", "scan", "pftn", "trees", "optim", "code",
             "local", "reader", "local2", "order", "match", "allo", "comm1", "table"]
    for source in original.iterdir():
        if source.is_file() and (source.suffix == ".c" or not source.suffix):
            shutil.copyfile(source, directory / source.name)
    source = directory / "code.c"
    text = source.read_text().replace("tmpfile", "tmpfp")
    text = text.replace("FILE *outfile = stdout;", "FILE *outfile;")
    text = text.replace('char *tmpname = "/tmp/pcXXXXXX";', 'char tmpname[] = "/tmp/pcXXXXXX";')
    text = text.replace("\tint dexit();", "\tint dexit();\n\toutfile = stdout;")
    source.write_text(text)
    source = directory / "scan.c"
    source.write_text(source.read_text().replace("dimtab[NULL]", "dimtab[0]"))
    source = directory / "local2.c"
    source.write_text(source.read_text().replace("maxtreg & =", "maxtreg &="))
    compiler = directory / "c68-host"
    argv = ["cc", "-O", "-w", "-Wno-implicit-int", "-Wno-implicit-function-declaration",
            "-Wno-return-mismatch", "-Wno-return-type", "-Wno-int-conversion",
            *(name + ".c" for name in names), "-o", compiler]
    ok, detail = regress.command(argv, directory / "build.log", cwd=directory)
    if not ok:
        raise RuntimeError("Cannot build original backend: " + detail)
    return compiler


def multifile(shared, build):
    directory = build / "multifile"
    directory.mkdir(exist_ok=True)
    objects = []
    for name in ("left", "right"):
        source = HERE / "common" / (name + ".c")
        assembly = directory / (name + ".az8")
        obj = directory / (name + ".b")
        for stage, argv, stdin, output in [
            ("compile", [TARGET / "cz8" / "cz8"], source.read_bytes(), assembly),
            ("assemble", [TARGET / "az8" / "az8", "-o", obj.name, assembly.name], None, None),
        ]:
            ok, detail = regress.command(argv, directory / (name + "-" + stage + ".log"),
                                         source=stdin, output=output, cwd=directory)
            if not ok:
                return {"multifile": {"status": "FAIL", "stage": stage, "detail": detail}}
        objects.append(obj)
    # A classic portable archive without a host-specific symbol index.
    data = objects[0].read_bytes()
    header = ("left.b/".ljust(16) + "0".ljust(12) + "0".ljust(6) + "0".ljust(6)
              + "100644".ljust(8) + str(len(data)).ljust(10) + "`\n").encode()
    archive = directory / "test.a"
    archive.write_bytes(b"!<arch>\n" + header + data + (b"\n" if len(data)&1 else b""))
    results = {}
    for name, inputs in [("multifile", objects), ("archive", [objects[1], archive])]:
        binary = directory / (name + ".bout")
        argv = [shared / "ldz8", "-x", shared / "crt0.b", "-R", "8", *inputs,
                shared / "exit.b", shared / "liblong.b", "-o", binary]
        ok, detail = regress.command(argv, directory / (name + "-link.log"), cwd=directory)
        if "ldz8: Undefined -" in detail:
            ok = False
        if not ok:
            results[name] = {"status": "FAIL", "stage": "link", "detail": detail.strip()}
            continue
        ok, detail = regress.command([HERE.parent / "run_emu", binary, "-e", "0"],
                                     directory / (name + "-run.log"), cwd=directory)
        results[name] = {"status": "PASS" if ok else "FAIL"}
        if not ok:
            results[name].update(stage="run", detail=detail.strip())
    return results


def main():
    # Build current tools and runtime via the existing isolated pipeline.
    subprocess.run([sys.executable, REGRESS / "run.py", "--case", "precedence"], check=True)
    shared = REGRESS / "build"
    build = HERE / "build"
    build.mkdir(exist_ok=True)
    m68k = build_m68k(build)
    results = {}
    original_results = {}
    for source in sorted(HERE.glob("*.c")):
        directory = build / source.stem
        directory.mkdir(exist_ok=True)
        original_argv = [m68k]
        ok68, detail68 = regress.command(original_argv, directory / "m68k.log",
                                         source=source.read_bytes(),
                                         output=directory / "m68k.s", cwd=directory)
        original_results[source.stem] = {"status": "COMPILED" if ok68 else "FAIL",
                                         "detail": detail68.strip()}
        assembly = directory / "test.az8"
        obj = directory / "test.b"
        binary = directory / "test.bout"
        stages = [
            ("compile", [TARGET / "cz8" / "cz8"], source.read_bytes(), assembly),
            ("assemble", [TARGET / "az8" / "az8", "-o", obj.name, assembly.name], None, None),
            ("link", [shared / "ldz8", "-x", shared / "crt0.b", "-R", "8", obj,
                      shared / "exit.b", shared / "liblong.b", shared / "libfloat.b",
                      shared / "softfp.b", "-o", binary], None, None),
            ("run", [HERE.parent / "run_emu", binary, "-e", "0"], None, None),
        ]
        for stage, argv, stdin, output in stages:
            ok, detail = regress.command(argv, directory / (stage + ".log"),
                                         source=stdin, output=output, cwd=directory)
            if stage == "link" and "ldz8: Undefined -" in detail:
                ok = False
            if not ok:
                results[source.stem] = {"status": "FAIL", "stage": stage, "detail": detail.strip()}
                break
        else:
            results[source.stem] = {"status": "PASS"}
        r = results[source.stem]
        print(r["status"], source.stem, r.get("stage", ""), r.get("detail", ""))
    extra = multifile(shared, build)
    results.update(extra)
    for name, r in extra.items():
        print(r["status"], name, r.get("stage", ""), r.get("detail", ""))
    (build / "m68k-results.json").write_text(json.dumps(original_results, indent=2) + "\n")
    print(f"68000 backend: {sum(r['status']=='COMPILED' for r in original_results.values())} "
          f"of {len(original_results)} probes compiled (no 68000 execution).")
    (build / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    failed = sum(r["status"] == "FAIL" for r in results.values())
    print(f"{len(results)-failed} PASS, {failed} FAIL; logs in {build}")
    return int(bool(failed))


if __name__ == "__main__":
    sys.exit(main())
