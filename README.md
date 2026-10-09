# Z8000 PCC

The compiler derives from the Motorola 68000 PCC backend in `68000/c68/`.
It targets K&R C, with 16-bit integers/pointers, 32-bit longs and IEEE floating
values. Z8000 Unix uses two native passes (`front` and `back`) to fit separate
64 KiB instruction and data spaces. `cz8` is the host one-pass build of the
same backend; `oz8` compacts generated assembly.

| Source | Purpose |
|---|---|
| `z8000/cz8/` | Compiler backend and generated parser |
| `z8000/ccz8.c` | Unix compiler driver |
| `z8000/oz8.c`, `c2z8.py` | Native optimizer and host comparison implementation |
| `z8000/crt0.az8`, `lib/` | Standalone startup, integer and IEEE software runtime |
| `z8000/test/` | Compiler execution, comparison, external, K&R and source-ratchet suites |
| `z8000/yacc/` | V7 yacc used to regenerate the parser |
| `pcc-tests/` | Upstream source for adapted tests |

The Z8000 toolchain uses **s.out only**. The historical az8 assembler, a.out
linker and b.out header are removed. Host and native compilation use the shared
`asz8k` assembler and `ldz8` linker in the `z8000_unix` repository. The tests
use those same sources, not an alternate test-only writer. Portable ASCII
archives contain s.out members. Default names such as `a.out` and `.b` suffixes
do not indicate a different binary format.

## Building and testing

In the Unix repository's `PCC-z8000` submodule, the suites find the parent
checkout automatically. For a separate PCC checkout, set `SOUT_ROOT` to the
absolute path of a `z8000_unix` checkout. Python suites use that environment
variable; make targets accept the same value as a make variable.

```sh
make -C z8000/cz8
make -C z8000/test test
make -C z8000/test regress-strict
make -C z8000/test compare68k
make -C z8000/test external
make -C z8000/test knr
make -C z8000/test optimizer
make -C z8000/lib
```

`cz8/cgram.c` is generated from `cgram.y`; regenerate through its makefile,
never by editing the output. The original 68000 backend is compiled for
comparison, but its generated code is not executed.

Current s.out pipeline checks pass 15 core programs, 87 strict regression
cases, 54 comparison cases and 58 external cases; all 58 K&R probes pass.
Optimizer checks cover comparison with the host implementation, saturated
jump-map execution and oversized-line rejection. The standalone Z8002 runner
accepts only resolved combined-space NONSEG s.out. It rejects historical
16/32-bit headers and unsupported split/SEG layouts. Unix integration tests
exercise split I/D and segmented machine assembly separately.

## Unix integration

Native compiler, tools/libc, userland, kernel and disk bootstrap rebuilding
is documented in the parent repository under `doc/development/native-rebuild.md`.
The same assembler/linker sources build on the host and in V7. Kernel/user C
uses the NONSEG ABI; SEG objects serve firmware and machine code, not a full
segmented C process ABI.

Unix floating operations use the separately mapped Zilog software EPU service.
Standalone compiler tests retain this repository's IEEE runtime so they can
execute without the Unix trap service. The static-result structure-return ABI
remains a limitation documented in the parent repository.
