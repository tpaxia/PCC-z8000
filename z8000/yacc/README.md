# Seventh Edition yacc

The parser generator that `cz8/cgram.c` is generated with. `y1.c`–`y4.c`,
`dextern` and `files` are unmodified copies of `/usr/src/cmd/yacc` from
Seventh Edition Unix. `yaccpar` is V7's `/usr/lib/yaccpar` with its five
`yydebug` lines wrapped in `#ifdef YYDEBUG`, which is the skeleton the
original `cgram.c` was generated with.

Nothing here is built by hand. `cz8/regen_cgram.py` copies these files to a
temporary directory, points `PARSER` at `yaccpar`, raises the table size from
`MEDIUM` to `HUGE` (the C grammar overflows the default), builds yacc with the
host compiler and runs it on `cgram.y`:

```sh
make -C z8000/cz8 cgram.c          # regenerate
make -C z8000/test parser-sync     # check that cgram.c is up to date
```
