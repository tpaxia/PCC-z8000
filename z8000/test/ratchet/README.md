# No-regression ratchet

The regression, comparison and external suites test constructs someone thought
to write a test for. This gate does the opposite job: it compiles a large body
of real K&R source that nobody wrote for the compiler, and fails when anything
that used to work stops working. It exists because two rounds of compiler fixes
passed every other suite while breaking the V7 shell and seven kernel files.

It is not a correctness or conformance claim. Nothing is executed. A file that
compiles here may still be miscompiled.

## What it checks

Each corpus file goes through `cpp | cz8 | az8`. Two things are compared with
the recorded baseline in `baseline/`:

| Check | Baseline file | Fails when |
| --- | --- | --- |
| Diagnostics | `<corpus>.diag.json` | a file that compiled no longer does, gains a warning, or no longer assembles |
| Assembly | `<corpus>.asm.json` | the generated assembly of a compiling file changes |

The assembly check is what catches silent code generation changes. A change is
not forbidden; it has to be looked at and accepted, and the accepted hashes are
part of the commit that caused them.

## Corpora

Defined in `corpora.json`.

| Corpus | Source | Files |
| --- | --- | --- |
| `v7-kernel` | `usr/sys/sys`, `usr/sys/dev`, `usr/sys/machine`, `usr/sys/conf` | 29 |
| `v7-sh` | `usr/src/cmd/sh` | 20 |
| `v7-cmd` | the rest of `usr/src/cmd` | 523 |
| `v7-libc` | `usr/src/libc` | 89 |
| `pcc-self` | this toolchain: `cz8`, `az8`, `lib`, `ldz8.c`, `ccz8.c`, `oz8.c` | 28 |

The V7 corpora live outside this repository, in the adapted V7 tree of the
`z8000_unix` project. The runner looks for it at `$V7_ROOT`, then `--v7-root`,
then `../v7z8000` beside this repository (its location when this repository is
a submodule of `z8000_unix`). A missing corpus is a failure, never a skip.

Many `v7-cmd` files do not compile: they need generated headers, `yacc` output,
PDP-11 assumptions, or exceed compiler table sizes. That is recorded, not
hidden. Such a file cannot regress; it can only start compiling.

## Running

```sh
make -C z8000/test ratchet       # this gate
make -C z8000/test gate          # every suite plus this gate
python3 z8000/test/ratchet/run.py --corpus v7-kernel --verbose
```

Result categories:

| Category | Meaning | Fails | Cleared by |
| --- | --- | --- | --- |
| `OK` | matches the baseline | no | |
| `IMPROVED` | fewer warnings, or now compiles | no | `--accept` |
| `REGRESSION` | worse than the baseline | yes | fixing the compiler |
| `ASM-CHANGED` | assembly differs from the recorded hash | yes | `--accept-asm` after review |
| `ASM-UNRECORDED` | compiles, no hash recorded yet | yes | `--accept-asm` after review |
| `STALE` | the source changed since the baseline | yes | `--accept` |
| `UNBASELINED` | new file in the corpus | yes | `--accept` |
| `GONE` | baseline entry with no file | yes | `--accept` |

`--accept` never records a regression. `--rebaseline` does, and should only be
used to create a baseline from a known-good compiler.

## Reviewing an assembly change

```sh
python3 z8000/test/ratchet/run.py --diff-against <git-rev>
```

builds `cz8` from that revision of this repository, compiles every corpus with
both compilers, and writes unified diffs of the assembly to
`build/diff/<corpus>/`. Read them, confirm they are what the change intended,
then run `--accept-asm`.

## Where the baselines came from

- **Diagnostics** were recorded with `cz8` built from `510a0f5`, the last
  revision known to build and boot the V7 kernel and shell:
  `run.py --cz8 build/ref-510a0f5/z8000/cz8/cz8 --rebaseline diag`
  (`--diff-against 510a0f5` builds that compiler).
- **Assembly hashes** were recorded with the compiler in the commit that
  introduced this ratchet. This is a starting snapshot for detecting change,
  not a statement that the code is right: 183 of those files produce different
  assembly from `510a0f5`, and those differences have not been reviewed.

The ratchet was written after a round of compiler changes passed every other
suite while breaking 57 corpus files (20 kernel, 19 shell, 18 commands) that
`510a0f5` compiled. Run against that broken compiler it reports exactly those
57 regressions.

## Limits

- Results depend on the host `cpp`. It is run with `-nostdinc -undef`, so only
  the V7 headers are used, but a different preprocessor may tokenise
  differently and make entries `STALE`.
- After its first error `cz8` recovers unreliably: later messages vary and it
  often dies on a signal, differently from run to run. Only the first error of
  a failing file is recorded, and the per-corpus count of such deaths is
  printed but not gated.
- Line numbers are part of a warning's identity, so editing a source file makes
  its entry `STALE` rather than silently matching.
