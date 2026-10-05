# External K&R compiler tests

This suite adds executable tests beyond the Unix V7 workload. The initial
selection contains 48 GCC torture cases, eight additional PCC cases, and two
derived PCC cases that isolate independent failures. On the current Z8000
toolchain, **all 58 pass**, after fixing the original 15 failures. Failures
remain visible and make the runner exit nonzero. The general regression suite
now passes 70 cases, including three additional floating runtime checks.

## Running the tests

From the repository root:

```sh
make -C z8000/test external
python3 z8000/test/external/run.py --case gcc-900409-1
python3 z8000/test/external/run.py --compare68k
```

The runner rebuilds the compiler, assembler, linker, emulator driver, and
integer/floating runtime through the existing regression setup. Every test
then goes through preprocessing, compilation, assembly, linking, and execution
on the Z8002 emulator. Success requires exit status zero. An unresolved linker
symbol is a failure even though this linker currently exits zero after reporting
it. Each emulator run has a two-million-cycle limit and each tool invocation
has a 20-second timeout.

Generated sources, assembly, binaries, stage logs, and `results.json` are kept
under the ignored `build/` directory. The checked-in `results.json` is the passing snapshot;
`results_before_fixes.json` preserves the initial 43-pass/15-failure result. Neither is an expectation list. There are no XFAIL exemptions.

`--compare68k` uses the host compatibility build in `../compare68k/run.py` to
compile the same preprocessed sources with the original backend. **56 of 58
compile** there. This comparison does not assemble, link, or execute 68000 code;
successful compilation does not establish correct 68000 behavior.

## Upstream sources and K&R adaptations

The GCC inputs come from the
[GCC 2.95 execute torture directory](https://github.com/gcc-mirror/gcc/tree/28971fe1a47e10822fa151d92d1e727f941dc4e8/gcc/testsuite/gcc.c-torture/execute),
pinned to commit `28971fe1a47e10822fa151d92d1e727f941dc4e8`. The first 100 C
files in directory name order were reviewed: 48 were selected and 52 were
excluded for documented language, target, runtime, or correctness reasons.
This is an initial selection, not coverage of the entire upstream suite.

Original GCC files are retained under `gcc/original/`, their K&R versions under
`gcc/kr/`, and the upstream GPLv2 notice in `gcc/COPYING`. The imported files
were verified against their upstream Git blob hashes.

PCC inputs refer to the existing `pcc-tests` submodule at commit
`bce11cbb345b0cf9f702ed57ba09704ea3d153f0`. Adapted cases are in `pcc/kr/`.
The manifest records each original path, original and adapted SHA256 hashes,
adaptations, and every exclusion from the reviewed GCC batch. The runner
verifies source hashes before execution; editing a test requires an explicit
manifest update after review.

Adaptations replace prototypes with old-style definitions, use K&R literal
spellings, and insert assignment spacing where this compiler recognizes the
historical `=op` compound assignment spelling. Local aggregate initializers
are expressed as assignments. Output-only PCC cases receive explicit result
checks. Plain `char` is signed on this target, so selected `signed char` tests
use `char`. The short-argument division test uses explicit casts to retain the
promotions implied by its original prototype.

The host compiler performs preprocessing only, with host-specific predefined
macros removed and no host headers. All executed expressions are compiled by
`cz8`; host integer widths are never used as the correctness oracle. This does
not test the historical target preprocessor. Selected tests do not depend on
ANSI macro stringification or token pasting.

Small K&R implementations of `abort`, `strcmp`, and `strcpy` are included only
when needed. `abort` terminates with status 99. The suite uses its own assembly
`exit` stub, which reads the stack argument before halting. The older regression
stub preserves R0 from `main` and cannot correctly observe direct calls to
`exit(status)` in upstream tests.

## Failures exposed before the fixes

| Cases | Observed failure | Comparison and scope |
| --- | --- | --- |
| `gcc-921112-1` | Invalid `ldir` operands when copying an eight-byte union through a pointer | Another executable reproduction of the large aggregate copy gap already found in the parity audit. The union contains a double but performs no floating arithmetic. |
| `pcc-namespace0`, `pcc-namespace1` | Parser rejects a typedef spelling reused as an enum constant or local object, with the same spelling also used for a tag/member | Both original 68000 and Z8000 compilers reject these sources. These are inherited frontend namespace gaps. |
| `pcc-optim002_case` | Assembler reports multiply defined symbols for identifiers differing only by case | In `az8/scan.c`, lowercase lookup accepts any defined symbol rather than restricting the fallback to registers. The uppercase symbol is consequently confused with its lowercase counterpart. This derived case removes the independent label collision. |
| `pcc-optim002_label` | Executes and exits with status 1 | A local label named `a` displaces the global variable `a` in the condition: generated code compares against the label address. The same wrong address appears in the original 68000 assembly. This derived case removes identifiers differing only by case. |
| `pcc-optim002` | Assembler reports multiply defined symbols | Original case contains both preceding defects. It remains unchanged. |
| Nine GCC floating cases | Eight compiler failures and one unresolved `fix` conversion helper | Covers float stores/comparisons, double division/multiplication/comparisons, indirect double calls, mixed arguments, and double-to-int conversion. All nine compile with the original 68000 backend. |

All cases in this table now pass. The compiler now has correct symbol namespaces,
the assembler restricts lowercase fallback to register names, and aggregate
copying handles the exact byte count. Floating code uses explicit conversions
and an integer-only IEEE runtime for arithmetic, comparisons, and updates.

The nine floating cases are `921013-1`, `921019-2`, `921124-1`, `921208-1`,
`930106-1`, `930603-1`, `930614-1`, `930614-2`, and `930628-1`.

Passing cases cover long masks, byte signedness and conversions, signed long
division from short arguments, pointer updates and initializers, nested calls,
structure arguments, small structure returns, unions, switches, loops, and
conditional expressions. Passing one case does not establish completeness for
that language feature.

SDCC and other suites have not been imported in this batch. Their compatible
cases can be added using the same provenance and execution requirements.
