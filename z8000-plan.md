# Z8000 PCC Port — Implementation Plan

## Current implementation status

This document preserves the original implementation plan below, including its
initial register choices and early gap assessments. The implemented ABI uses
R13 as the frame pointer and R15 as the stack pointer; scalar returns use R0,
RR0, or R0–R3 for word, long/float, or double values respectively. Float arguments
are promoted to double under K&R default argument promotions.

The subsequent compiler audit and fixes now pass 70 general regression cases,
54 comparison probes and 58 external GCC/PCC K&R cases, with no expected-failure
exemptions. The software floating runtime implements basic arithmetic,
comparisons, conversions and compound/prefix/postfix updates, with nearest/even
rounding. Earlier statements below that these routines are unimplemented refer
to the original plan. Current results, build commands and remaining limitations
are maintained in [README.md](README.md) and the linked suite reports.

## Context

Port the Portable C Compiler (PCC) from the Motorola 68000 backend (`68000/c68/`) to the Zilog Z8002 (nonsegmented, 16-bit flat address space). The result is a full cross-compilation toolchain: compiler backend (`cz8`), assembler (`az8`), linker (`ldz8`), and driver (`ccz8`).

## Target: Z8002 Nonsegmented

- 16 x 16-bit GPRs (R0-R15), with overlapping byte (RH/RL0-7), long-pair (RR0-RR14 even), quad (RQ0-RQ12 multiples of 4)
- R0 **cannot** be used for indirect/indexed addressing
- R15 = hardware stack pointer (grows down)
- 16-bit flat address space (64KB)
- Hardware MULT (16x16→32), DIV (32/16→16q+16r)
- No hardware FPU
- 8 addressing modes: R, IR, IM, DA, X, RA, BA, BX

## Type Sizes

| Type | Bits | Notes |
|------|------|-------|
| char | 8 | |
| short | 16 | = int |
| int | 16 | native word |
| long | 32 | register pair (RRn) |
| pointer | 16 | = int |
| float | 32 | software library |
| double | 64 | software library |

## Calling Convention

- R15 = SP, R14 = FP (frame pointer)
- R0 = return value (word), RR0 = return value (long)
- Arguments on stack, right-to-left, 16-bit aligned
- Callee-saved: R4-R7 (data), R10-R13 (address)
- Caller-saved: R0-R3, R8-R9
- Prologue: `push @.sp,.r14 / ld .r14,.sp / sub .sp,#framesize`
- Epilogue: `ld .sp,.r14 / pop .r14,@.sp / ret`

## Register Classes (PCC model)

- **SAREG** (R0-R7): data/arithmetic registers. R0-R3 scratch, R4-R7 reg vars
- **SBREG** (R8-R13): address/pointer registers. R8-R9 scratch, R10-R13 reg vars
- R14 (FP), R15 (SP): SBREG, not allocatable as temporaries

## Directory Structure

```
z8000/
├── cz8/          # compiler backend
│   ├── macdefs       # type sizes, register constants
│   ├── mac2defs      # register defs, shapes
│   ├── code.c        # prologue/epilogue, switch
│   ├── local.c       # pass 1 tree transforms
│   ├── local2.c      # pass 2: adrput, zzzcode, register names
│   ├── order.c       # register allocation, SU numbers
│   ├── table.c       # instruction templates (biggest file)
│   ├── Makefile
│   └── (copied unchanged: cgram.y cgram.c trees.c pftn.c optim.c
│        scan.c xdefs.c comm1.c allo.c match.c reader.c
│        mfile1 mfile2 manifest common)
├── az8/          # assembler
│   ├── inst.h        # instruction dispatch numbers
│   ├── mical.h       # assembler defs
│   ├── init.c        # instruction table init
│   ├── ins.c         # Z8000 instruction encoding (biggest file)
│   ├── scan.c        # input scanning (register names, addressing syntax)
│   ├── sym.c         # symbol table
│   ├── error.c       # errors
│   ├── rel.c         # relocation (b.out format)
│   ├── ps.c          # pseudo-ops
│   ├── sdi.c         # span-dependent instruction resolution
│   └── Makefile
├── ccz8.c        # driver (based on cc68.c)
├── ldz8.c        # linker (based on ld68.c, minimal changes)
└── crt0.az8      # C runtime startup
```

## Implementation Steps

### Phase 1: Compiler Backend — Machine Definitions & Framework

1. **Create `z8000/cz8/` directory and copy machine-independent files** from `68000/c68/`:
   `cgram.y cgram.c trees.c pftn.c optim.c scan.c xdefs.c comm1.c allo.c match.c reader.c mfile1 mfile2 manifest common`

2. **Write `macdefs`**: SZINT=16, SZPOINT=16, SZLONG=32, ARGINIT=32, AUTOINIT=0, STKREG=14, ARGREG=14, MINRVAR=4, MAXRVAR=7. Define BACKAUTO, BACKTEMP, ONEPASS.

3. **Write `mac2defs`**: R0-R7 (SAREG), R8-R13 (SBREG), FP=14, SP=15. REGSZ=16, TMPREG=FP. Shapes: SCCON (-128..127), SICON (0..32767), S8CON (1..8). No NESTCALLS.

4. **Fix 3 compile errors** inherited from 68000 sources:
   - `code.c`: rename `tmpfile` variable (clashes with stdlib)
   - `local2.c:277`: fix `& =` → `&=`
   - `scan.c:105`: fix `dimtab[NULL]` → `dimtab[0]`

### Phase 2: Compiler Backend — Machine-Dependent Code

5. **Write `local.c`** (adapt from 68000):
   - `clocal()`: NAME→OREG for AUTO/PARAM via R14. PCONV trivial (pointers=int=16 bit)
   - `ctype()`: pass-through (LONG ≠ INT on Z8000, unlike 68000)
   - `cisreg()`: true for all scalar types
   - `notoff()`: offsets -32768..32767 legal for R1-R15, reject R0

6. **Write `code.c`** (adapt from 68000):
   - `bfcode()`: Z8000 prologue (push FP / ld FP,SP / sub SP,#framesize / save regs via LDM)
   - `efcode()`: struct return handling, branch to retlab
   - `genswitch()`: table switch using indexed LD + JP, or binary search using CP/JR
   - `branch()`, `deflab()`, `locctr()`: change `.bra`→`jr`, labels `.L%d`

7. **Write `local2.c`** (major rewrite):
   - `rnames[]`: `.r0`..`.r13`, `.r14`, `.sp`
   - `rstatus[]`: SAREG|STAREG for R0-R7, SBREG|STBREG for R8-R13, SBREG for R14,R15
   - `adrput()`: NAME→symbol, ICON→`#val`, REG→register name, OREG→`offset(.rN)` or `@.rN`
   - `zzzcode()`: ZB (type suffix b/w/l), ZC (call cleanup), ZI (conditional branch), ZS (struct copy via LDIR), ZP (arg stack tracking), Z- (push)
   - `hopcode()`: map operators to Z8000 mnemonics (add/sub/and/or/xor/sla/sra)
   - `ccbranches[]`: `jr eq/ne/lt/le/gt/ge/ult/ule/ugt/uge`
   - `hardops()`: long mul/div/mod → library calls; all float/double → library calls
   - `szty()`: returns 2 for LONG/ULONG/FLOAT, 1 otherwise (DOUBLE handled by hardops, never in regs)
   - `rmove()`: `ld .rN,.rM` for word, `ldl .rrN,.rrM` for long

8. **Write `order.c`** (adapt from 68000):
   - `rallo()`: FORCE→R0|MUSTDO, MUL/DIV→even register for pair result
   - `offstar()`: direct address computations into INTBREG (R8-R13)
   - `indexreg()`: returns true for R1-R15, false for R0
   - `shumul()`: STARNM only (no STARREG — Z8000 has no auto-increment)
   - `argsize()`: int/pointer = 2 bytes, long = 4 bytes, double = 8 bytes (NOT 4 for everything like 68000)
   - `sucomp()`: adjust SU numbers for szty=2 longs

### Phase 3: Instruction Templates

9. **Write `table.c`** — the largest single piece of work. Template categories:

   **ASSIGN**: `ld`/`ldb`/`ldl` for reg-reg, reg-mem, mem-reg, clear with `clr`/`clrb`
   **OPLTYPE** (loads): `ld`/`ldb`/`ldl` into SAREG/SBREG, FORARG templates using `push @.sp,.rN`
   **OPLOG** (compare): `cp`/`cpb`/`cpl` + `ZI` conditional branch; `test`/`testb` for compare-with-zero
   **ASG PLUS/MINUS**: `add`/`addb`/`addl`, `sub`/`subb`/`subl`; `inc`/`dec` for small constants
   **ASG OR/AND/ER**: `or`/`and`/`xor` + byte variants
   **ASG LS/RS**: `sla`/`sra`/`sll`/`srl` with immediate count or register shift via `sda`/`sdl`
   **MUL**: `mult .rrN, src` (16x16→32, extract low word). Needs even-register constraint
   **DIV/MOD**: `exts .rrN` + `div .rrN, src`. Quotient in Rd+1, remainder in Rd
   **UNARY MINUS/COMPL**: `neg`/`negb`, `com`/`comb`
   **INCR/DECR**: `inc`/`dec` with immediate 1-16, or `add`/`sub` for larger
   **SCONV**: char→int via `extsb`, uchar→int via `and #0xFF`, int→long via `exts .rrN`
   **UNARY MUL**: indirect load `ld .rN, @.rM`
   **UNARY CALL**: `call symbol` or `call @.rN`
   **CCODES**: `ld .rN,#1` + conditional `ld .rN,#0`
   **STASG**: struct assignment via `ldir @.rN, @.rM, .r0` for large, individual LD for small
   **FLD**: bit field extract/insert via shifts and masks

### Phase 4: Assembler

10. **Create `z8000/az8/` directory**, copy framework files from `68000/a68/`:
    `sym.c error.c rel.c ps.c sdi.c Makefile`

11. **Write `inst.h`**: dispatch numbers for ~110 Z8000 instructions (ld, ldb, ldl, add, sub, cp, call, ret, jr, jp, push, pop, mult, div, exts, extsb, sla, sra, ldir, etc.)

12. **Write `mical.h`**: operand types (t_reg, t_ireg, t_immed, t_da, t_x, t_ra, t_ba, t_bx), instruction size limits

13. **Write `init.c`**: op_codes[] mapping mnemonic strings to dispatch numbers, register name initialization (`.r0`-`.r15`, `.rr0`-`.rr14`, `.rh0`-`.rh7`, `.rl0`-`.rl7`, `.sp`)

14. **Adapt `scan.c`**: recognize Z8000 register names and `@` for indirect addressing

15. **Write `ins.c`** (major work): Z8000 instruction encoding. The Z8000 has a fairly regular encoding:
    - Bits 15-8: opcode, Bits 7-4: dst reg, Bits 3-0: src reg (for register-register)
    - Multi-word for immediate/address operands
    - Encoding helpers: `encode_ld()`, `encode_alu()`, `encode_shift()`, `encode_call()`, `encode_jr()`, `encode_jp()`, `encode_push()`, etc.

16. **Adapt `sdi.c`**: JR (2 bytes, ±256 range) → JP (4 bytes) expansion

### Phase 5: Driver, Linker, Runtime

17. **Write `ccz8.c`** (adapt from cc68.c): change pass names (cz8, az8, oz8), file extensions (.az8), predefined macros (-Dz8000 -Dz8002)

18. **Adapt `ldz8.c`** from ld68.c: minimal changes — b.out format is machine-independent. Change default load address, use RWORD (16-bit) relocations for pointers

19. **Write `crt0.az8`**: C runtime startup — zero BSS, set up stack, call `main`, call `exit`

20. **Write runtime library stubs**: `lmul`, `ldiv`, `lrem` (32-bit arithmetic), float/double library routines (can be stubbed initially)

### Phase 6: Testing & Verification ✓

21. ✓ **Build the compiler**: `cd z8000/cz8 && make`
22. ✓ **Build the assembler**: `cd z8000/az8 && make`
23. ✓ **Test with simple programs**: `main(){return 42;}` compiles and assembles end-to-end
24. ✓ **Test arithmetic**: int add/sub/mul/div, long arithmetic, long shifts, type conversions — 11 test programs pass
25. ✓ **Test control flow**: if/else, while, for, switch all work (dense table jump and sparse binary search)
26. ✓ **Test pointers and arrays**: basic indirection works; bitfield read/write tested
27. ✓ **Test function calls**: word and long args work; struct return works (STCALL via genscall/gencall)
28. ✓ **End-to-end emulator execution**: 15 test programs compile → assemble → link → execute on Z8002 emulator and return correct results. Test driver (`z8000/test/run_emu.cpp`) loads b.out binaries into the `z8000_emu` emulator, runs them, and checks R0 return value.

**Core tests** (7): hello (return 42), arith (recursive factorial), control (loops/pointers/arrays/structs), switch (dense table + sparse binary search), bitfield (read/write), shift (word + long shifts), larith (32-bit multiply/divide/modulo).

**pcc-tests adapted** (8, from `pcc-tests/tests/c/`): pcc_math (int div/mod/xor/or/and), pcc_cmp (systematic signed + unsigned comparisons), pcc_struct (struct assignment from local/global/static), pcc_structret (struct return from function), pcc_union (union pass by value + address-of parameter), pcc_ptr (pointer arrays + indexing), pcc_scope (variable scoping + shadowing), pcc_optim (constant folding + dead code + loop with multiply).

#### Bugs found and fixed during Phase 6:
- Assembler: `sopcode()` fallback, `!` comment char, `exts` size, indexed MULT/DIV, memory-immediate CP
- Assembler: INC/DEC count field off-by-one (stored n instead of n-1; validated against z8k-coff-as)
- Assembler: SRL/SRA/SRLL/SRAL right-shift count not negated (Z8000 uses signed count; validated against z8k-coff-as)
- Assembler: `.zerow` pseudo-op missing — compiler emits `.zerow N` for zero-initialized word-sized static data; assembler had `.zerol` (zero longs) but not `.zerow`; added `Zerow()` handler
- Runtime: crt0.az8 referenced `_main`/`_exit` but compiler emits `main`/`exit` (no underscore prefix)
- Compiler: INTEMP mem-to-mem, INCR/DECR operand order, callee-saved register saves, `tymatch()` missing LONG case, SCONV template shapes, LONG↔ULONG template, `cbgen()` double-free, `genswitch()` R0 indirect bug, bitfield assign memory-dest AND/OR

#### Resolved during investigation (not actual gaps):
- **Pointer SCONV** — PCONV nodes eliminated by `clocal()` in local.c before reaching template matching; no templates needed
- **Switch statements** — `genswitch()` already existed in code.c with both table jump (dense) and binary search (sparse) paths; only needed R0 indirect fix
- **Struct arguments (STARG)** — already handled in `order.c:genargs()` which pushes struct words onto stack
- **Struct-valued returns (STCALL)** — already handled: `genscall()` delegates to `gencall()` (rewrites to UNARY CALL); `efcode()` copies return value to static area
- **Bitfield reads** — already worked via shift-right + AND mask in register; only bitfield writes had bugs (memory-dest AND/OR)
- **Long shifts** — Z8000 has hardware long shifts (`slal`/`sral`/`srll`/`sdal`/`sdll`); templates were simply missing from table.c
- **Assembler pseudo-ops** — `.=.+N` (space), `.even` (align), and symbol assignment already supported; compiler doesn't generate `.space`/`.align`/`.fill`. Full pseudo-op set: `.byte`, `.word`, `.long`, `.text`, `.data`, `.bss`, `.globl`, `.comm`, `.even`, `.ascii`, `.asciz`, `.zerol`, `.zerow`
- **Variadic functions** — stack-based calling convention works naturally; added `include/varargs.h` header
- **Linker** — works for current use; weak symbols and validation are low-priority features no PCC port implements

### Phase 7: Remaining Work

**High priority — blocks real programs:**
- ✓ Long multiply/divide runtime library (`lmul`, `ldiv`, `lrem`, `ulmul`, `uldiv`, `ulrem` plus assignment variants `almul`, `aldiv`, `alrem`, `aulmul`, `auldiv`, `aulrem`) — implemented in `z8000/lib/arith.az8`; tested via `z8000/test/larith.c` with signed/unsigned multiply, divide, modulo, fast-path (two-DIV) and slow-path (binary long division) unsigned division
- Float/double software library — all float/double ops emit calls to `fadd`, `fsub`, `fmul`, `fdiv`, `fneg`, `float` (int→float), `fix` (float→int) plus assignment variants (`afadd`, etc.); none are implemented

**Medium priority — assembler encoding bugs (not used by compiler, affect hand-written assembly):**
- BIT/SET/RES register mode — `bit_op()` uses IR-mode base opcodes (0x2700) but never adds 0x8000 for R mode; `bitb rl0,#0` encodes as `2680` instead of correct `A680`
- DJNZ offset encoding — uses only 4 bits for a 7-bit displacement field

**Low priority — cosmetic / completeness:**
- Linker "Undefined" warnings — prints warnings for local labels (`.lbzloop`, `.l12`, `_f1`) that are actually resolved; output works correctly; cosmetic only
- Standard library headers — only `varargs.h` exists; no stdio.h, stdlib.h, string.h (acceptable for bare-metal cross-compiler)

## Key Technical Risks

1. **Register pair allocation for longs**: PCC's `allo.c` allocates consecutive registers for szty=2. Z8000 requires even-register pairs. The natural register numbering (R0=0, R2=2, etc.) helps, but odd-starting pairs must be prevented. May need a small `allo.c` tweak or enforce via `rallo()` hints.

2. **No auto-increment addressing**: The 68000 heavily uses `reg@+`. Remove all STARREG templates. Structure copy uses Z8000's `LDIR` block transfer instead.

3. **16-bit int changes propagate widely**: argument sizes, constant ranges, type conversion paths, SU numbers all change from the 68000 baseline.

4. **BA/BX only with LD**: Base Address and Base Index modes are only available with Load instructions, not with arithmetic. This limits which templates can use OREG for source operands in ADD/SUB/etc. — need to load into register first.
