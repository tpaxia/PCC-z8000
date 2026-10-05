int (*funp)();
int seen;
test(s) char *s; { if (*s == 'd') seen |= 1; else if (*s == 'i') seen |= 2; else abort(); return 0; }
main() { funp = test; test("direct"); (*funp)("indirect"); if (seen != 3) abort(); return 0; }
