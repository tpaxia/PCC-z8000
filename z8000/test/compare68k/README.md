# Comparison of the 68000 and Z8000 compiler ports

This audit initially compared the complete checked-in `68000/c68` and `z8000/cz8` compilers at PCC revision `fc3991d`, including shared parsing and tree passes, target types, register allocation, instruction templates, initialization, calling conventions, aggregates, and the driver/linker/runtime boundary. It found concrete non-floating port failures in addition to the incomplete floating implementation.

The current suite reports **54 PASS and 0 FAIL** after fixing the failures found in this audit. The initial result at `fc3991d` was 28 passes and 26 failures: 11 non-floating failures and all 15 floating probes. `results_before_fixes.json` preserves that result; `results.json` records the passing run. The failure descriptions below refer to the initial audit. The separate general regression suite now has 70 passing cases, and the external GCC/PCC suite has 58.

## How the comparison was performed

The audit compares every shared C/grammar/header source file and examines the changed target routines. `source_comparison.json` records which shared files were identical at `fc3991d`. The instruction tables were inspected by operation, type, addressing shape and register needs rather than treating different assembly strings as missing features.

Run from the repository root:

```sh
python3 z8000/test/compare68k/run.py
```

The runner builds the current Z8000 tools and runtime, then compiles, assembles, links and executes each Z8000 probe on the Z8002 emulator. It also builds a host executable of the original 68000 backend from isolated copies, using only compatibility changes: renaming the conflicting `tmpfile` variable, initializing `outfile` at runtime, making the temporary filename writable, replacing the `NULL` array index with zero, and joining the historical `& =` spelling into `&=`. Original source files are not edited.

The 68000 backend compiles **51 of the 52 C probes**; `double_increment` fails in the original compiler and now passes on Z8000. Its emitted assembly is retained for comparison. The 68000 assembly/runtime was **not executed**, so successful compilation is not a claim that its entire historical toolchain works. The two additional probes check Z8000 multi-file linking and archive extraction.

Current complete logs, assembly and results are under `build/`. `results.json` in this directory records the concise observed snapshot. A failing discovery run exits nonzero.

## Non-floating port gaps found before the fixes

| Area and reproductions | Z8000 evidence | Difference from the 68000 compiler |
| --- | --- | --- |
| Register long parameters — `register_long_param` | Returning `0x12345678L` from a function with a `register long` parameter returns the wrong value. | [code.c](../../cz8/code.c), `bfcode`, emits one 16-bit `ld` into the allocated pair and omits its second word. The original `bfcode` loads a full 32-bit register. |
| Long to pointer conversion — `pointer_long_cast` | Casting stored `0x12345678L` to a 16-bit pointer selects `0x1234`, not the low word `0x5678`. | [local.c](../../cz8/local.c), `clocal(PCONV)`, discards the conversion and changes the operand type. On the original target both long and pointer are 32 bits; on Z8000 this loses the required truncation and address adjustment. |
| Byte to pointer conversion — `pointer_byte_cast` | Casting a stored negative char to a pointer does not preserve the sign extension expected by the probe. | Original `clocal(PCONV)` explicitly routes byte/short conversions through `SCONV`; Z8000 removed that path on the assumption that all relevant values already have pointer width. Eight-bit source values still need widening. Changing the stored byte node to pointer type also causes a word load that reads an adjacent byte. |
| Clearing a bitfield — `bitfield_clear` | `x.b=0` leaves a previously nonzero field unchanged. | [table.c](../../cz8/table.c), zero-field `ASSIGN`, emits `and ...,#Z~`. This complements the zero RHS into all ones. The original uses `#N`, the complement of the field mask. |
| Variable byte shifts — `shift_byte` | `char`/`unsigned char` compound shifts by an `int` variable produce `no table entry for op REG`. | Original byte-shift templates accept register counts. Z8000 byte templates accept only constant counts; word/long register-count templates do not cover byte compound assignments. |
| Large aggregate copies — `struct_copy_large`, `struct_copy_pointer` | Six-byte copies emit rejected forms such as `ldir @dst,@src,r0` and `ldir @@r9,@@r8,r0`. | [local2.c](../../cz8/local2.c), `zzzcode('S')`, substituted a block-copy instruction without materializing valid addresses. The original copies through valid target addressing modes. A second source-level defect uses the byte size as the word count; simply repairing the syntax would still copy twice the requested bytes. |
| Large aggregate returns — `struct_return_large` | Returning a six-byte structure eventually emits rejected aggregate-copy assembly. | Both `efcode` return handling in [code.c](../../cz8/code.c) and the caller's `zzzcode('S')` use the new block-copy path. The return copy also passes bytes as the `ldir` word count. This remains blocked before an execution test can establish all consequences. |
| Long switch expressions — `switch_long` | `0x10001L` and `0x20001L` select the same case. | Shared [cgram.y](../../cz8/cgram.y), `switchpart`, forces the expression to `INT`; [code.c](../../cz8/code.c), `genswitch`, compares one word. This was sufficient where original int and long were both 32 bits. Z8000 now truncates distinct long values to the same 16-bit value. |
| Unsigned switch ranges — `switch_unsigned` | A sparse switch spanning 1 through 65535 routes low values into the wrong subtree. Calls explicitly pass 16-bit unsigned arguments. | `genswitch`/`genbinary` retain signed `gt` branches while case constants are sorted as positive host longs. Above 32767 their signed 16-bit interpretation changes; the ordering and comparisons disagree. The original 32-bit comparisons do not encounter that sign boundary for these values. |

## Inherited behavior and gaps in the original baseline

`bitfield_signed` fails its signed-value expectation on Z8000. Inspection of the original emitted assembly shows the same field extraction algorithm: shift and mask without sign extension. This is inherited behavior, not a newly omitted Z8000 template. Plain `int` bitfield signedness can depend on the historical dialect, so this probe does not alone establish a violation of its original language contract.

`double_increment` fails compilation in both backends. The original compiler is not a complete correctness oracle.

Both linkers report undefined symbols but finish with `exit(0)`: see [ldz8.c](../../ldz8.c), `main` and `middle`, and `68000/ld68.c`. The test runners explicitly treat these diagnostics as failures. This inherited behavior can otherwise make incomplete images look like successful builds.

## Floating support differences before the fixes

The original instruction table contains 32-bit float loads, stores, pushes, and float/double conversion entries, plus double condition handling. Z8000 lacks the corresponding complete entries. `float_copy`, `float_double`, `double_float`, `double_compare`, `double_truth` and `double_increment` fail compilation. The 68000 backend compiles the first five; its increment failure is shared.

The original `hardops` and `hardconv` delegate arithmetic and integer/floating conversion to named runtime routines. The Z8000 backend retains those names, but its runtime at `fc3991d` provided only double addition (`fadd`). `double_sub`, `double_mul`, `double_div`, `double_neg`, `double_compound`, `int_double`, `long_double`, `double_int` and `double_long` fail linking with missing helpers. No implementations of the original 68000 floating helpers were found in this repository; their call sites do not prove an available working runtime.

The original float/double conversion templates move/truncate 32-bit pieces and zero the extra word. They must not be copied as a correct IEEE widening/narrowing implementation. The Z8000 IEEE representation, 16-bit registers, new argument widths and integer widths require explicit numerical conversions and compatible runtime entry points.

## Driver and optional toolchain gaps

The original tree includes the `o68` assembly optimizer. The Z8000 tree contains no `oz8` counterpart, but [ccz8.c](../../ccz8.c) still sends `-O` output to `/lib/oz8` and treats a failing optimizer invocation as an error. This is a source-confirmed unported optional stage; the discovery suite does not install or execute the full driver/preprocessor environment.

Profiling still emits `call mcount` and selects profiling startup variants, but this Z8000 tree provides neither `mcount` nor those startup objects. No profiling execution is claimed. Likewise, the driver retains historical installed tool/include/library paths; direct compiler tests do not validate a complete installation or libc.

Assembler instruction encodings are a separate audit surface. Existing README notes flag register `BIT/SET/RES` and `DJNZ` encoding gaps. They are not compiler feature losses established by this comparison, and the current compiler normally does not emit those instructions. The valid addressing and block-count errors in aggregate copies above are compiler emission defects.

## Features exercised successfully

The passing probes cover signed word/long division and remainder for both divisor signs; long carry/borrow; 16-bit and 32-bit unsigned constant-folding wrap boundaries; indirect long loads/stores; word/long updates through incrementing pointers; mixed-width signed comparisons; register byte parameters; nested word/long calls; long function pointers; register pressure and preservation across calls; large structure arguments; unsigned bitfield widths and zero-width separators; enum/static initialization; aggregate padding; union byte order; integer varargs; textual inline assembly; and branches across a large function. Multi-file external/common symbols, file-local statics, relocations and archive extraction also pass their probes.

At the original audit revision, parsing was largely preserved: `cgram.y`, generated `cgram.c`, `optim.c`, `match.c`, `comm1.c`, `common`, `manifest`, `mfile1`, `mfile2`, and `xdefs.c` were identical. Scanner differences include fixes for malformed constants and typedef shadowing. ANSI prototypes, modern qualifiers and other modern syntax absent from both grammars are shared language boundaries rather than lost Z8000 features. Old assignment spellings mean `x=-7`, `p=&v`, and `p=*q` can have different meanings; these probes use spaces to avoid that ambiguity.

Changing int/pointer widths to 16 bits, preserving 32-bit longs, using register pairs/quads, lowering unsupported memory arithmetic, replacing auto-increment addressing, changing stack slots and using an a.out object format are intentional target adaptations. Losing the original hardware-specific short-multiply optimization or auto-increment instruction forms is an optimization difference when equivalent C still works.

## Scope and remaining checks

This is a source comparison plus targeted execution, not exhaustive equivalence. The passing 70-case suite, 58 external cases, and these 54 probes do not establish a complete compiler. For example, alternate rounding modes, compound updates through every storage shape, exhaustive conversion/storage combinations, extreme register pressure, overlapping aggregates, every assembler instruction, profiling, full driver/preprocessor integration, and the complete V7 userland remain outside verified coverage.

The integer/pointer/aggregate failures found in this audit could affect kernel and userland code when those constructs occurred; their reproductions now pass. Determining which affect a particular Unix image requires matching the actual build sources/options and emitted objects; this comparison does not rebuild or certify the Unix kernel.
