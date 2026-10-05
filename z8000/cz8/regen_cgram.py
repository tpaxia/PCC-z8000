#!/usr/bin/env python3
"""Regenerate cgram.c from cgram.y with the Seventh Edition yacc.

cgram.c is generated; never edit it. It is the only parser: the host build
compiles it, and being K&R C it is also what cz8 itself can compile.

V7 yacc and its parser skeleton live in ../yacc; see the README there.

    regen_cgram.py            write cgram.c
    regen_cgram.py --check    fail if cgram.c is not what cgram.y generates
"""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
YACC = HERE.parent / "yacc"
CFLAGS = ["-w", "-std=gnu89", "-Wno-implicit-int", "-Wno-implicit-function-declaration",
          "-Wno-return-type", "-Wno-int-conversion", "-Wno-incompatible-pointer-types"]


def build_yacc(work):
    for name in ("y1.c", "y2.c", "y3.c", "y4.c", "dextern", "files", "yaccpar"):
        shutil.copy(YACC / name, work / name)
    files = (work / "files").read_text()
    # Point yacc at our skeleton and give it the table sizes this grammar needs.
    for old, new in (('# define PARSER "/usr/lib/yaccpar"', '# define PARSER "%s"' % (work / "yaccpar")),
                     ("# define MEDIUM", "# define HUGE")):
        if old not in files:
            sys.exit("unexpected V7 yacc 'files' header: %r not found" % old)
        files = files.replace(old, new)
    (work / "files").write_text(files)
    built = subprocess.run(["cc", *CFLAGS, "-o", "yacc", "y1.c", "y2.c", "y3.c", "y4.c"],
                           cwd=work, capture_output=True)
    if built.returncode != 0:
        sys.exit("V7 yacc does not build:\n" + built.stderr.decode()[-2000:])
    return work / "yacc"


def generate():
    with tempfile.TemporaryDirectory(prefix="pcc-v7yacc-") as directory:
        work = Path(directory)
        yacc = build_yacc(work)
        # yacc records the input name in #line directives, so run it on "cgram.y".
        shutil.copy(HERE / "cgram.y", work / "cgram.y")
        ran = subprocess.run([yacc, "cgram.y"], cwd=work, capture_output=True)
        output = work / "y.tab.c"
        text = (ran.stdout + ran.stderr).decode()
        if ran.returncode != 0 or "fatal" in text or not output.exists():
            sys.exit("V7 yacc failed:\n" + text)
        return output.read_bytes()


def main():
    generated = generate()
    target = HERE / "cgram.c"
    if "--check" in sys.argv[1:]:
        if not target.exists() or target.read_bytes() != generated:
            print("cgram.c is out of date: run 'make -C z8000/cz8 cgram.c' (never edit it by hand)")
            return 1
        print("cgram.c matches cgram.y")
        return 0
    target.write_bytes(generated)
    print("wrote cgram.c (%d bytes)" % len(generated))
    return 0


if __name__ == "__main__":
    sys.exit(main())
