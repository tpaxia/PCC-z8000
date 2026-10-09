# Z8002 compiler audit and regression tests

The current strict suite reports **86 PASS, 0 XFAIL, 0 XPASS, 0 FAIL**, both
with and without the native assembly optimizer. The counts below record
earlier stages of the audit.

This suite exercises the general K&R C compiler, beyond the subset used by Unix V7. The audit found failures in lexical validation, typedef handling, scalar conversions, long operations, initialization, register allocation, and the assembler/linker pipeline. It adds 45 C execution/code-emission cases, 3 assembly isolation cases, and 7 diagnostic cases. It also rebuilds and runs the existing tests.

On upstream `main` at **510a0f5**, before these local fixes, 47 cases passed and 15 failed. All 15 failures are now fixed. That initial fixed suite reported **67 PASS, 0 XFAIL, 0 XPASS, 0 FAIL**, including five additional boundary/runtime/storage/vector cases and the pre-existing `larith.c`. The setup also checks the integer-only double adder against 100,000 deterministic host IEEE additions.

The fixes cover static long initializers, signed/unsigned byte widening, live shift counts, long complement, typedef shadowing, malformed hex constants, unsigned word division across the sign-bit boundary, and the missing long runtime helpers. Double addition now has big-endian IEEE constants, four-register values, memory copies, arguments/returns, spills and a software arithmetic helper. The subsequent fixes add software subtraction, multiplication, division, negation, ordered/unordered comparisons, float/double and integer conversions, compound updates, and floating increments. The expanded suite reports **70 PASS, 0 XFAIL, 0 XPASS, 0 FAIL**.

The separate [external K&R suite](../external/README.md) adds 58 GCC/PCC cases and now reports **58 PASS, 0 FAIL**, after fixing inherited frontend namespace defects, an assembler symbol lookup defect, aggregate copies, and floating failures. Run it with `make -C z8000/test external`. Its failures are reported directly and are not given expected-failure exemptions.

The original audit used 434a41a and found 30 passes and 32 failures across 23 problem groups. The table below preserves that history. Upstream 510a0f5 fixed 18 cases; the first regression fixes resolved the remaining 15. `expectations.json` is now empty.

## Running the suite

From the repository root:

```sh
make -C z8000/test regress
make -C z8000/test regress-strict
python3 z8000/test/regress/run.py --case long_zero --case pointer_init
```

Normal mode succeeds only when every case either passes or reproduces its documented failure stage and diagnostic/result signature. A new failure is FAIL. A fixed expected failure is XPASS and also fails the command, so its expectation must be removed. Strict mode returns nonzero for any remaining expected failure. Each command rebuilds the selected C, assembly, and linked images; it never consumes the existing `.bout` files.

Assembly and full stage logs are retained under `build/<case>/`. Compiler and assembler binaries, the emulator library/driver, and a private linker are built as needed. Commands have a 20-second timeout; execution has a one-million-cycle limit, raised to ten million for `float_vectors`, `float_ops_vectors`, and `float_general`, which perform software arithmetic. `--build-dir PATH` selects another artifact directory. Case artifact filenames stay short because the assembler's pathname buffers are only 32 bytes.

The runner assembles `lib/arith.az8`, `lib/float.az8`, and the target-compiled `lib/softfp.c`. `make -C z8000/lib` also builds these runtime objects for linking with user programs. The 15 core programs, including `test/larith.c`, are included in the current 70-case run. The runner treats unresolved-symbol diagnostics as a link failure even though the linker currently exits zero.

`expectations.json` records observed failures, `diagnostics.json` specifies required rejection diagnostics, and `codegen.json` checks initializer emission before assembler/data-layout errors can mask compiler errors. A reject test must exit unsuccessfully with the intended diagnostic; an internal compiler error is not accepted as a valid rejection.

## Language and compiler model

The lexer and grammar in [scan.c](../../cz8/scan.c) and [cgram.y](../../cz8/cgram.y) implement an extended K&R dialect: implicit int, old-style function definitions and unspecified argument lists, typedefs, enums, structures/unions, bitfields, aggregate assignment and calls, casts, and the usual expression/control-flow operators. Declarations precede statements in each block. Old-style initialization without `=` and old assignment spellings are deliberately accepted with warnings. The scanner also recognizes `asm` and `fortran` extensions.

Function prototypes, `const`, `volatile`, the `signed` keyword, and modern declaration syntax are not this compiler's language contract. The prototype diagnostic case records the boundary rather than treating lack of ANSI C as a bug. The test sources use K&R definitions, block comments, and `L` constants. Signed right shift is tested as arithmetic, consistent with the existing port's instruction templates and tests.

[macdefs](../../cz8/macdefs) defines 8-bit char, 16-bit int/short/pointers, 32-bit long/float, and 64-bit double, with 16-bit aggregate alignment. Names are limited to eight characters by `NCHNAM`; label names now use their own namespace. Plain int bitfields are signed and are sign-extended on extraction. Preprocessing is a separate toolchain step; these fixtures feed preprocessed-style C directly to `cz8`.

Pass 1 builds and optimizes typed expression trees in `trees.c`, `pftn.c`, `optim.c`, and `local.c`. This port runs both passes in one executable. Pass 2 orders trees and allocates registers in `order.c`/`allo.c`, selects templates in `table.c`, and expands target escapes and operands in `local2.c`. Long values occupy register pairs; doubles occupy aligned four-register groups. The assembler and linker are included in the execution tests because incorrect encoding or object layout can otherwise look like compiler miscompilation.

## Original audit findings at 434a41a

| ID | Reproduction | Evidence and source explanation |
| --- | --- | --- |
| B01 | `bitfield_init` | `{5,17,200}` emits `.word 0` rather than 45512. `local.c`'s `incode()` packs into the low 16 bits but outputs `word >> 16`; `vfdzero()` has the same word-emission pattern. |
| B02 | `char_init` | `{1,42}` emits zero under `a:`; its packed value 298 appears under the following string's label. The same packing/emission state is shifted by one word and leaks across initializers. |
| B03 | `long_init` | `65538L` emits two `.word 65538` directives instead of high word 1 and low word 2. The long INIT template in `table.c` uses `CL` twice without splitting. |
| B04 | `char_postinc` | Returns 1 even with runtime initialization. Generated byte loads use `r0` rather than `rl0`, followed by word sign extension, and increment/decrement use word instructions (`inc a,#1`). The existing postincrement templates omit the byte suffix for the update. Byte encoding also contributes; see B22. |
| B05 | `char_to_long`, `knr_calls` | A char value entering a long expression triggers `expression causes compiler loop`. `table.c` has word-to-long conversions but no char/uchar-to-long conversion. The K&R call fixture fails while compiling the callee expression, before call execution. |
| B06 | `control_flow` | `sum += i + j` emits `add -22(r14),r0`, rejected by the assembler. Z8000 register/memory arithmetic requires a register destination; general assignment templates allow memory destinations. |
| B07 | `pointer_init`, `declarators`, `initializers` | A static pointer/address initializer produces an object rejected with `invalid symbol id in relocation command` and `bad relocation size`. Likely contributors are append-mode data/relocation streams in `az8/rel.c` and host-sized layout calculations; see B21. Exact relocation repair remains to be established. |
| B08 | `dynamic_count` | `x >> n` negates a live `register int n`; the following `n != 4` fails. Right-shift templates negate `AR` without requiring a scratch register or restoring it. |
| B09 | `dynamic_shift`, `asm_dynamic_shift` | Arithmetic dynamic right shift gives the wrong value. `az8/ins.c` assigns SDA word/long the SDL opcodes `B303`/`B307`, and SDL the SDA opcodes `B30B`/`B30F`. This is visible independently of C in the assembly fixture. |
| B10 | `float_add` | Double constant assignment already triggers a compiler loop, before addition or linking. Missing software floating-point runtime is a known gap, but there are backend failures before runtime resolution too. |
| B11 | `long_bits` | Long AND fails with `no table entry for op REG`. The bitwise assignment templates cover word and byte types, not long register pairs. Later OR/XOR assertions await a fix to the first failure. |
| B12 | `long_unary`, `long_complement` | Long negation and complement trigger compiler loops. Unary templates only cover words/bytes. These are independent fixtures with runtime-assigned inputs. |
| B13 | `long_postinc` | Long postincrement lowers to memory add/sub instructions rejected by the assembler. The test crosses the low-word carry/borrow boundary 65535 to 65536. Correct full-width update and preservation of the old value remain unverified. |
| B14 | `long_truth`, `long_zero` | Long truth tests and comparisons against zero fail with `no table entry for op REG`. OPLTYPE/FORCC has word/byte tests but no long test. Variable-to-variable long comparison does pass. |
| B15 | `register_calls` | Long addition emits `adc r0,#1`, which cannot be assembled. Carry arithmetic templates allow immediate sources although ADC requires a register source. The fixture cannot reach its register-preservation checks yet. |
| B16 | `register_long` | First register-long assignment fails with `illegal register pair freed`. `pftn.c` allocates one descending register per scalar, initially odd R7, while long values require an even pair. `cisreg()` currently accepts LONG/ULONG. |
| B17 | `typedef_basic` | An enum field is stored with `ld` but compared using `ldb/cpb` at the same address, and returns 2. The generated store/load widths disagree. The exact frontend/optimizer conversion responsible still needs tracing. |
| B18 | `typedef_scope` | A nested `int item;` cannot shadow an outer typedef named `item`: `illegal type combination` and `syntax error`. The declaration grammar and typedef-sensitive scanner misclassify the redeclared name. |
| B19 | `uchar_to_int` | Generated zero-extension mask is `and r0,#0x`, not `#0xFF`. `match.c` interprets uppercase `F` in the template as an expansion control character; literal hex digits must be escaped or replaced with a decimal constant. |
| B20 | `unsigned_div` | `65535 / 3` returns the wrong quotient. The template emits `subl rr0,rr0; ld r1,r1`, clearing the dividend before attempting to reload it. Even after fixing that, using signed DIV with unsigned divisors above 32767 needs separate validation. |
| B21 | `word_init`, `asm_word_data` | Even a plain `.word 1234` loses its initialized value through the pipeline and returns 1. The removed historical assembler and linker opened data streams with `"a"`, then seek to an intended section offset; append writes ignore the seek. Their removed `b.out.h` also used host `sizeof` for a format serialized as fixed 32-bit fields: on LP64 the header struct is 64 bytes and relocation struct 16 bytes, while serialized records are 32 and 8 bytes. The current s.out pipeline passes these probes. |
| B22 | `asm_byte_regs` | `ldb rl0,#0x56` fails to update the low byte of R0. `az8/ins.c`'s `regfield()` maps RL0-RL7 to fields 0-7, the same as RH0-RH7; low-byte register fields require bit 3. |
| B23 | `reject_empty_hex`, `reject_bad_hex_prefix` | `0x` and `12x34` are accepted with status zero. `scan.c` doesn't require a hex digit, and its hex-prefix condition fails to reject multi-digit prefixes. (`09` is not an error: K&R gives the digits 8 and 9 the octal values 10 and 11, and the K&R probe `l241_octal89` checks that.) |

## Coverage and next investigations

The added passing cases cover precedence and assignment associativity; short-circuit side effects; conditional/comma evaluation; unevaluated `sizeof`; word/long conversions and truncation; unsigned constant width; signed/unsigned long comparisons at word and sign boundaries; function pointers; pointer subtraction and struct scaling; aggregate byte copies; and bitfield postincrement/compound update. Existing tests add recursion, switch dispatch, structure return/pass-by-value, unions, scope, arithmetic, shifts, and the optional long runtime.

New boundary checks exercise every signed/unsigned byte value when widening to long, unsigned division/remainder and compound updates at sign-bit and quotient boundaries, and double values in pointers, arrays, structures and nested calls. The 67 target addition vectors check exact result bits for both signs, cancellation, nearest/even ties, overflow, infinity, subnormal transitions, NaN quieting/payloads and exponent gaps. The floating runtime uses nearest/even rounding; it does not implement floating exception flags or alternate rounding modes.

Further useful coverage includes exhaustive boundary comparisons across all storage shapes, signed division/remainder across all sign combinations, long add/sub carry/borrow, every conversion width/signedness pair, compound assignment through side-effecting pointers, nested aggregate calls, register pressure and spills, anonymous/zero-width bitfields, multi-file extern/static/common symbols, archive extraction, and driver/preprocessor behavior. The new `float_ops_vectors` checks 57 bit-exact multiplication/division/subtraction results; `float_convert_vectors` checks 25 IEEE widening/narrowing cases including ties, subnormals, signed zeros, infinity, and NaNs. `float_general` checks default argument promotions, signed/unsigned conversion boundaries, all four basic compound operators, compound rounding at a binary32 tie, prefix/postfix increments and decrements, floating truth, and unordered comparisons. Host runtime checks additionally validate 100,000 additions, 20,121 cases for each new binary operation, 20,000 conversions in each floating direction, integer boundaries, and all six relations for exceptional values. Passing on this emulator is evidence for this pipeline, not proof of hardware correctness; assembler encoding tests against an independent assembler would strengthen it.

For B09 and B22, an independent manual check with `z8k-coff-as -z8002` confirmed `sda r0,r1` as `B30B 0100`, `sdl r0,r1` as `B303 0100`, and `ldb rl0,#0x56` as the compact low-byte encoding `C856`. This assembler emits `B303 0100` for SDA and a high-byte long-form load `2000 0056` for RL0. GNU's toolchain is not required to run the regression suite.

The parser `cz8/cgram.c` is generated from `cgram.y` by the Seventh Edition yacc in `z8000/yacc` (`make -C z8000/cz8 cgram.c`); it is the only parser and is never edited by hand. `lib/gen_float.py` regenerates the stack-ABI wrappers in `lib/float.az8`.
