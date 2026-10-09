#!/usr/bin/env python3
"""K&R coverage probes: one small self-checking program per group of language
rules, indexed by section of the C Reference Manual (K&R, first edition,
Appendix A).

Each probe names the items it checks in a `KNR:` header line. sections.json
lists every item, so an item with no probe shows up as UNCOVERED. Results are
compared with status.json: a probe recorded as passing must keep passing.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

HERE = Path(__file__).resolve().parent
TEST = HERE.parent
TARGET = TEST.parent
sys.path.insert(0, str(TEST))
from toolchain import AS, LD, setup_commands
PROBES = HERE / "probes"
CC_FLAGS = ["-O", "-w", "-Wno-implicit-int", "-Wno-implicit-function-declaration",
            "-Wno-int-conversion", "-Wno-return-mismatch"]


def run(argv, *, stdin=None, cwd=None):
    try:
        r = subprocess.run([str(a) for a in argv], input=stdin, capture_output=True,
                           timeout=60, cwd=cwd)
    except subprocess.TimeoutExpired:
        return 1, b"", b"timeout"
    return r.returncode, r.stdout, r.stderr


def first_line(data):
    text = data.decode(errors="replace").strip().splitlines()
    return text[0].strip() if text else ""


def load_probes(known_items):
    probes = []
    for path in sorted(PROBES.glob("*.c")):
        text = path.read_text()
        m = re.search(r"KNR:\s*([^\n*]+)", text)
        if not m:
            sys.exit("%s: no KNR: header" % path.name)
        items = m.group(1).split()
        unknown = [i for i in items if i not in known_items]
        if unknown:
            sys.exit("%s: unknown items: %s" % (path.name, " ".join(unknown)))
        probes.append({"name": path.stem, "path": path, "items": items,
                       "reject": "EXPECT-ERROR" in text})
    return probes


def build_runtime(build):
    """Assemble crt0 and the runtime library once; return the object list."""
    cz8, az8 = TARGET / "cz8" / "cz8", AS
    setup = [["make", "-C", TARGET / "cz8"], *setup_commands(),
             ["make", "-C", TEST, "run_emu"],
             ["cp", LD, build / "ldz8"]]
    for argv in setup:
        rc, out, err = run(argv)
        if rc != 0:
            sys.exit("setup failed: %s\n%s" % (" ".join(map(str, argv)), (out + err).decode()[-2000:]))
    rc, soft, err = run([cz8], stdin=(TARGET / "lib" / "softfp.c").read_bytes())
    if rc != 0:
        sys.exit("softfp.c does not compile:\n" + err.decode())
    sources = [("crt0", (TARGET / "crt0.az8").read_bytes()),
               ("exit", (TARGET / "lib" / "exit.az8").read_bytes()),
               ("liblong", (TARGET / "lib" / "arith.az8").read_bytes()),
               ("libfloat", (TARGET / "lib" / "float.az8").read_bytes()),
               ("csv", (TARGET / "lib" / "csv.az8").read_bytes()),
               ("softfp", soft)]
    objects = []
    for name, data in sources:
        (build / (name + ".az8")).write_bytes(data)
        rc, out, err = run([az8, "-c", "-o", name + ".b", name + ".az8"], cwd=build)
        if rc != 0:
            sys.exit("runtime %s does not assemble:\n%s" % (name, (out + err).decode()))
        objects.append(build / (name + ".b"))
    return cz8, az8, objects


def run_probe(probe, cz8, az8, objects, build):
    """Return 'PASS' or 'FAIL <stage>: <detail>'."""
    work = build / probe["name"]
    work.mkdir(exist_ok=True)
    rc, asm, err = run([cz8], stdin=probe["path"].read_bytes())
    (work / "compile.log").write_bytes(err)
    errors = [l for l in err.decode(errors="replace").splitlines()
              if "line " in l and "warning:" not in l]
    if probe["reject"]:
        return "PASS" if rc != 0 and errors else "FAIL compile: invalid program accepted"
    if rc != 0 or errors:
        detail = errors[0] if errors else "cz8 exit %d" % rc
        return "FAIL compile: " + re.sub(r"^[^,]*, ", "", detail)
    (work / "t.az8").write_bytes(asm)
    rc, out, err = run([az8, "-c", "-o", "t.b", "t.az8"], cwd=work)
    if rc != 0:
        return "FAIL assemble: " + first_line(out + err)
    rc, out, err = run([build / "ldz8", "-x", objects[0],  "t.b", *objects[1:],
                        "-o", "t.sout"], cwd=work)
    text = (out + err).decode(errors="replace")
    if rc != 0:
        return "FAIL link: " + first_line(out + err)
    rc, out, err = run([TEST / "run_emu", "t.sout", "-e", "0", "-c", "20000000"], cwd=work)
    (work / "run.log").write_bytes(out + err)
    if rc == 0:
        return "PASS"
    m = re.search(r"R0=(-?\d+)", (out + err).decode(errors="replace"))
    return "FAIL run: " + ("check %s failed" % m.group(1) if m else first_line(out + err))


def write_matrix(sections, probes, results):
    by_item = {}
    for p in probes:
        for item in p["items"]:
            by_item.setdefault(item, []).append(p["name"])
    rows, totals = [], {"PASS": 0, "FAIL": 0, "UNCOVERED": 0}
    for s in sections:
        rows.append("\n### %s %s\n" % (s["id"], s["title"]))
        if s.get("note"):
            rows.append(s["note"] + "\n")
        rows.append("| Item | Rule | Probe | Result |\n| --- | --- | --- | --- |")
        for key, text in s["items"].items():
            names = by_item.get(s["id"] + "." + key, [])
            if not names:
                state = "UNCOVERED"
            elif all(results[n] == "PASS" for n in names):
                state = "PASS"
            else:
                state = "FAIL"
            totals[state] += 1
            rows.append("| %s | %s | %s | %s |" % (key, text, ", ".join("`%s`" % n for n in names) or "", state))
    failing = ["| `%s` | %s |" % (p["name"], results[p["name"]][5:]) for p in probes
               if results[p["name"]] != "PASS"]
    total = sum(totals.values())
    head = [
        "# K&R coverage matrix",
        "",
        "Generated by `run.py --accept`; do not edit. Items are the rules of the C",
        "Reference Manual (K&R, first edition, Appendix A), plus the additions the",
        "V7 compiler made after the book. Only the probes in `probes/` are counted",
        "here; the regression, comparison and external suites are not.",
        "",
        "| Items | Count |", "| --- | --- |",
        "| Probed and passing | %d |" % totals["PASS"],
        "| Probed and failing | %d |" % totals["FAIL"],
        "| Not probed | %d |" % totals["UNCOVERED"],
        "| Total | %d |" % total,
        "",
        "%d probes, %d passing." % (len(probes), sum(1 for p in probes if results[p["name"]] == "PASS")),
    ]
    if failing:
        head += ["", "## Failing probes", "", "| Probe | Failure |", "| --- | --- |", *failing]
    head += ["", "## By section"]
    (HERE / "MATRIX.md").write_text("\n".join(head + rows) + "\n")
    return totals


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--probe", action="append", help="run the named probe only (repeatable)")
    ap.add_argument("--accept", action="store_true",
                    help="record the current results in status.json and regenerate MATRIX.md")
    ap.add_argument("--build-dir", type=Path, default=HERE / "build")
    args = ap.parse_args()

    sections = json.loads((HERE / "sections.json").read_text())
    known = {s["id"] + "." + k for s in sections for k in s["items"]}
    probes = load_probes(known)
    if args.probe:
        unknown = set(args.probe) - {p["name"] for p in probes}
        if unknown:
            ap.error("unknown probe: " + ", ".join(sorted(unknown)))
        probes = [p for p in probes if p["name"] in args.probe]
    build = args.build_dir.resolve()
    build.mkdir(parents=True, exist_ok=True)
    cz8, az8, objects = build_runtime(build)

    status_path = HERE / "status.json"
    recorded = json.loads(status_path.read_text()) if status_path.exists() else {}
    results, counts, bad = {}, {}, False
    for p in probes:
        result = run_probe(p, cz8, az8, objects, build)
        results[p["name"]] = result
        before = recorded.get(p["name"])
        if before is None:
            kind = "NEW"
        elif result == "PASS":
            kind = "PASS" if before == "PASS" else "FIXED"
        else:
            kind = "REGRESSION" if before == "PASS" else "KNOWN"
        counts[kind] = counts.get(kind, 0) + 1
        bad = bad or kind in ("REGRESSION", "NEW")
        if kind != "PASS":
            print("%-10s %-22s %s" % (kind, p["name"], "" if result == "PASS" else result[5:]))
    print(", ".join("%d %s" % (n, k) for k, n in sorted(counts.items())))

    if args.accept:
        if args.probe:
            ap.error("--accept needs a full run")
        regressions = [n for n, r in results.items() if recorded.get(n) == "PASS" and r != "PASS"]
        if regressions:
            print("not accepting: %s passed before and fails now" % ", ".join(regressions))
            return 1
        status_path.write_text(json.dumps(results, indent=1, sort_keys=True) + "\n")
        totals = write_matrix(sections, probes, results)
        print("wrote status.json and MATRIX.md: %(PASS)d items pass, %(FAIL)d fail, %(UNCOVERED)d not probed" % totals)
        return 0
    if bad:
        print("\nKNR FAILED: a recorded pass now fails, or a probe has no recorded status (--accept records it)")
        return 1
    print("\nKNR OK (%d known failures)" % counts.get("KNOWN", 0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
