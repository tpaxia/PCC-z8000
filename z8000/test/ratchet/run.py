#!/usr/bin/env python3
"""No-regression ratchet: compile real K&R source corpora and compare with a baseline.

Every corpus file is preprocessed, compiled with cz8 and assembled with az8.
Two things are compared with the recorded baseline:

  diagnostics  errors, warnings and assembler status must not get worse
  assembly     generated code must not change unless the change is accepted

Nothing is executed. See README.md for the workflow.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import difflib
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

HERE = Path(__file__).resolve().parent
TEST = HERE.parent
TARGET = TEST.parent
REPO = TARGET.parent
BASELINE = HERE / "baseline"
DIAG = re.compile(r'^(?:"(?P<file>[^"]*)"[^,]*)?, line (?P<line>\d+): (?P<msg>.*)$')
CPPFLAGS = ["-nostdinc", "-undef", "-Dz8000", "-Dz8002"]
TIMEOUT = 60
FAILING = ("REGRESSION", "ASM-CHANGED", "ASM-UNRECORDED", "STALE", "UNBASELINED", "GONE")


def sha(data):
    return hashlib.sha256(data).hexdigest()[:16]


def run(argv, *, stdin=None, cwd=None):
    """Return (returncode, stdout, stderr); returncode is None on timeout."""
    try:
        r = subprocess.run([str(a) for a in argv], input=stdin, capture_output=True,
                           timeout=TIMEOUT, cwd=cwd)
    except subprocess.TimeoutExpired:
        return None, b"", b"timeout"
    return r.returncode, r.stdout, r.stderr


def source_key(preprocessed):
    """Hash of the preprocessed tokens, independent of line markers and spacing."""
    lines = [b" ".join(l.split()) for l in preprocessed.split(b"\n")
             if l.strip() and not l.lstrip().startswith(b"#")]
    return sha(b"\n".join(lines))


class Corpus:
    def __init__(self, spec, roots):
        self.name = spec["name"]
        self.root = roots[spec["root"]]
        self.base = self.root / spec["base"]
        self.spec = spec
        self.roots = roots

    def files(self):
        found = set()
        for pattern in self.spec["globs"]:
            found.update(p for p in self.base.glob(pattern) if p.is_file())
        excluded = set()
        for pattern in self.spec.get("exclude", []):
            excluded.update(self.base.glob(pattern))
        return sorted(p.relative_to(self.base).as_posix() for p in found - excluded)

    def includes(self, rel):
        """Include directories for one file, relative to the corpus base."""
        out = []
        for item in self.spec.get("include", []):
            path = item.replace("{dir}", str((self.base / rel).parent))
            for key, root in self.roots.items():
                path = path.replace("{%s}" % key, str(root))
            out.append(os.path.relpath(Path(self.base, path), self.base))
        return out

    def display(self, name):
        """Normalise a file name from a cpp line marker."""
        if not name:
            return "?"
        path = Path(os.path.normpath(self.base / name))
        for root in self.roots.values():
            try:
                return path.relative_to(root).as_posix()
            except ValueError:
                pass
        return name


def compile_one(corpus, rel, cz8, az8, build):
    """Preprocess, compile and assemble one file. Returns (record, asm bytes)."""
    rec = {"src": None, "status": "ok", "errors": [], "warnings": [], "az8": "skipped", "asm": None,
           "crashed": False}
    argv = ["cpp", *CPPFLAGS, *("-I" + i for i in corpus.includes(rel)), rel]
    rc, pre, err = run(argv, cwd=corpus.base)
    if rc != 0:
        lines = [l for l in err.decode(errors="replace").splitlines() if "error" in l]
        rec["status"] = "cpp"
        rec["errors"] = ["cpp: " + (lines[0].strip() if lines else "failed")]
        return rec, None
    rec["src"] = source_key(pre)
    rc, out, err = run([cz8], stdin=pre)
    asm = []
    for line in (out + b"\n" + err).decode(errors="replace").splitlines():
        m = DIAG.match(line)
        if not m:
            continue
        text = "%s:%s: %s" % (corpus.display(m["file"]), m["line"], m["msg"])
        (rec["warnings"] if m["msg"].startswith("warning:") else rec["errors"]).append(text)
    asm = b"\n".join(l for l in out.split(b"\n") if not DIAG.match(l.decode(errors="replace")))
    # After a first error cz8's recovery is unreliable: later messages vary and
    # it often dies on a signal, differently from run to run. Only the first
    # error is stable, so that is all that is kept for a failing file.
    rec["crashed"] = rc is not None and rc < 0
    if rc is None:
        rec["status"] = "timeout"
    elif rec["errors"] or rc > 0:
        rec["status"] = "error"
    elif rc < 0:
        rec["status"] = "crash"
        rec["errors"] = ["cz8 killed by signal %d with no diagnostic" % -rc]
    if rec["status"] != "ok":
        rec["errors"] = rec["errors"][:1]
        rec["warnings"] = []
    rec["warnings"].sort()
    if rec["status"] != "ok":
        return rec, None
    rec["asm"] = sha(asm)
    # az8 keeps file names in a 32-byte buffer, so use short names in its cwd.
    stem = "f" + sha(rel.encode())[:10]
    work = build / corpus.name
    (work / (stem + ".az8")).write_bytes(asm)
    rc, out, err = run([az8, "-o", stem + ".b", stem + ".az8"], cwd=work)
    rec["az8"] = "ok" if rc == 0 else "fail"
    if rc != 0:
        (work / (stem + ".log")).write_bytes(out + err)
    return rec, asm


def classify(base_diag, base_asm, rec):
    """Return (category, detail lines) for one file against its baseline."""
    if base_diag is None:
        return "UNBASELINED", []
    if base_diag["src"] != rec["src"]:
        return "STALE", ["source changed since the baseline was recorded"]
    if base_diag["status"] != "ok":
        # Already failing: it cannot regress, only start compiling.
        if rec["status"] != "ok":
            return "OK", []
    detail = []
    if base_diag["status"] == "ok" and rec["status"] != "ok":
        detail += ["status ok -> " + rec["status"]] + rec["errors"]
    if base_diag["az8"] == "ok" and rec["az8"] != "ok" and rec["status"] == "ok":
        detail.append("az8 ok -> " + rec["az8"])
    if base_diag["status"] == "ok":
        detail += sorted(set(rec["warnings"]) - set(base_diag["warnings"]))
    if detail:
        return "REGRESSION", detail
    recorded = (base_asm or {}).get("asm") if (base_asm or {}).get("src") == rec["src"] else None
    if rec["asm"] is not None:
        if base_asm is not None and base_asm.get("src") != rec["src"]:
            return "STALE", ["source changed since the assembly baseline was recorded"]
        if recorded is None:
            return "ASM-UNRECORDED", ["compiles, but no assembly hash is recorded"]
        if recorded != rec["asm"]:
            return "ASM-CHANGED", ["%s -> %s" % (recorded, rec["asm"])]
    better = ((base_diag["status"] == "ok" and len(rec["warnings"]) < len(base_diag["warnings"]))
              or (base_diag["status"] != "ok" and rec["status"] == "ok")
              or (base_diag["az8"] == "fail" and rec["az8"] == "ok"))
    return ("IMPROVED" if better else "OK"), []


def load(path):
    return json.loads(path.read_text()) if path.exists() else {}


def save(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=1, sort_keys=True) + "\n")


def diag_entry(rec):
    return {k: rec[k] for k in ("src", "status", "errors", "warnings", "az8")}


def build_reference(rev, build):
    """Build cz8 from another revision of this repository; return its path."""
    ref = build / ("ref-" + rev)
    cz8 = ref / "z8000" / "cz8" / "cz8"
    if not cz8.exists():
        ref.mkdir(parents=True, exist_ok=True)
        # Newer revisions generate the parser with the in-tree yacc.
        paths = ["z8000/cz8"]
        if subprocess.run(["git", "-C", REPO, "cat-file", "-e", rev + ":z8000/yacc"],
                          capture_output=True).returncode == 0:
            paths.append("z8000/yacc")
        archive = subprocess.run(["git", "-C", REPO, "archive", rev, *paths],
                                 capture_output=True, check=True).stdout
        subprocess.run(["tar", "-x", "-C", ref], input=archive, check=True)
        made = subprocess.run(["make", "-C", cz8.parent], capture_output=True)
        if made.returncode != 0 or not cz8.exists():
            sys.exit("cannot build reference cz8 at %s:\n%s" % (rev, made.stderr.decode()[-2000:]))
    return cz8


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--v7-root", type=Path, default=os.environ.get("V7_ROOT"),
                    help="adapted V7 tree (default: $V7_ROOT, else ../v7z8000 beside this repository)")
    ap.add_argument("--corpus", action="append", help="run the named corpus only (repeatable)")
    ap.add_argument("--cz8", type=Path, default=TARGET / "cz8" / "cz8", help="compiler under test")
    ap.add_argument("--az8", type=Path, default=TARGET / "az8" / "az8")
    ap.add_argument("--build-dir", type=Path, default=HERE / "build")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--no-asm", action="store_true", help="skip the assembly comparison")
    ap.add_argument("--accept", action="store_true",
                    help="record IMPROVED, STALE, UNBASELINED and GONE entries; never regressions")
    ap.add_argument("--accept-asm", action="store_true",
                    help="record new assembly hashes for ASM-CHANGED and ASM-UNRECORDED files")
    ap.add_argument("--rebaseline", choices=["diag", "asm", "all"],
                    help="overwrite the baseline with the current results, regressions included")
    ap.add_argument("--diff-against", metavar="REV",
                    help="also compile with cz8 built from git revision REV and write assembly diffs")
    ap.add_argument("--verbose", action="store_true", help="list every file, not only the exceptions")
    args = ap.parse_args()

    v7 = Path(args.v7_root) if args.v7_root else REPO.parent / "v7z8000"
    roots = {"v7": v7.resolve(), "pcc": REPO}
    specs = json.loads((HERE / "corpora.json").read_text())
    if args.corpus:
        unknown = set(args.corpus) - {s["name"] for s in specs}
        if unknown:
            ap.error("unknown corpus: " + ", ".join(sorted(unknown)))
        specs = [s for s in specs if s["name"] in args.corpus]
    for spec in specs:
        if not (roots[spec["root"]] / spec["base"]).is_dir():
            sys.exit("corpus %s: %s not found. Set V7_ROOT or pass --v7-root; a missing "
                     "corpus is a failure, not a skip." % (spec["name"], roots[spec["root"]] / spec["base"]))
    for tool in (args.cz8, args.az8):
        if not tool.exists():
            sys.exit("%s not built" % tool)
    build = args.build_dir.resolve()
    reference = build_reference(args.diff_against, build) if args.diff_against else None

    failed = False
    for spec in specs:
        corpus = Corpus(spec, roots)
        (build / corpus.name).mkdir(parents=True, exist_ok=True)
        files = corpus.files()
        with ThreadPoolExecutor(args.jobs) as pool:
            results = list(pool.map(lambda f: compile_one(corpus, f, args.cz8, args.az8, build), files))
        current = {f: rec for f, (rec, _) in zip(files, results)}
        diag_path = BASELINE / (corpus.name + ".diag.json")
        asm_path = BASELINE / (corpus.name + ".asm.json")
        base_diag, base_asm = load(diag_path), load(asm_path)

        counts, report = {}, []
        for f in files:
            rec = current[f]
            shown = dict(rec, asm=None) if args.no_asm else rec
            cat, detail = classify(base_diag.get(f), base_asm.get(f), shown)
            counts[cat] = counts.get(cat, 0) + 1
            if cat != "OK" or args.verbose:
                report.append((cat, f, detail))
        for f in sorted(set(base_diag) - set(files)):
            counts["GONE"] = counts.get("GONE", 0) + 1
            report.append(("GONE", f, ["in the baseline, no longer in the corpus"]))

        clean = sum(1 for r in current.values() if r["status"] == "ok" and r["az8"] == "ok")
        crashed = sum(1 for r in current.values() if r["crashed"])
        print("== %s: %d files, %d compile and assemble%s" % (
            corpus.name, len(files), clean,
            ", cz8 died on a signal in %d (not gated: varies between runs)" % crashed if crashed else ""))
        for cat, f, detail in report:
            print("%-14s %s" % (cat, f))
            for line in detail[:6]:
                print("                 " + line)
            if len(detail) > 6:
                print("                 ... %d more" % (len(detail) - 6))
        print("   " + ", ".join("%d %s" % (n, c) for c, n in sorted(counts.items())))
        if any(counts.get(c) for c in FAILING):
            failed = True

        if reference:
            out = build / "diff" / corpus.name
            out.mkdir(parents=True, exist_ok=True)
            with ThreadPoolExecutor(args.jobs) as pool:
                refs = list(pool.map(lambda f: compile_one(corpus, f, reference, args.az8, build), files))
            same = differ = 0
            for f, (rec, asm), (rrec, rasm) in zip(files, results, refs):
                if asm is None or rasm is None:
                    continue
                if asm == rasm:
                    same += 1
                    continue
                differ += 1
                text = difflib.unified_diff(rasm.decode(errors="replace").splitlines(True),
                                            asm.decode(errors="replace").splitlines(True),
                                            args.diff_against + "/" + f, "current/" + f)
                (out / (f.replace("/", "__") + ".diff")).write_text("".join(text))
            print("   vs %s: %d identical, %d differ (diffs in %s)" % (args.diff_against, same, differ, out))

        # Baseline updates.
        new_diag, new_asm = dict(base_diag), dict(base_asm)
        for cat, f, _ in report if not args.rebaseline else [("ALL", f, []) for f in files]:
            rec = current.get(f)
            if args.rebaseline in ("diag", "all") or (args.accept and cat in ("IMPROVED", "STALE", "UNBASELINED")):
                new_diag[f] = diag_entry(rec)
            if args.rebaseline in ("asm", "all") or (args.accept_asm and cat in ("ASM-CHANGED", "ASM-UNRECORDED")) \
                    or (args.accept and cat == "STALE" and f in base_asm):
                if rec["asm"] is not None:
                    new_asm[f] = {"src": rec["src"], "asm": rec["asm"]}
                else:
                    new_asm.pop(f, None)
            if cat == "GONE" and args.accept:
                new_diag.pop(f, None)
                new_asm.pop(f, None)
        if args.rebaseline:
            keep = set(files)
            new_diag = {f: v for f, v in new_diag.items() if f in keep} if args.rebaseline != "asm" else new_diag
            new_asm = {f: v for f, v in new_asm.items() if f in keep} if args.rebaseline != "diag" else new_asm
        if new_diag != base_diag:
            save(diag_path, new_diag)
            print("   wrote " + str(diag_path.relative_to(HERE)))
        if new_asm != base_asm:
            save(asm_path, new_asm)
            print("   wrote " + str(asm_path.relative_to(HERE)))

    if args.rebaseline or args.accept or args.accept_asm:
        print("\nBaseline updated; run again without flags to check.")
        return 0
    print("\n" + ("RATCHET FAILED" if failed else "RATCHET OK"))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
