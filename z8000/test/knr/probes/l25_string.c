/* KNR: 2.5.array-null 2.5.escapes 2.5.continuation 2.5.static */
char *keep() { return "kept"; }
main()
{
	char *p;
	if (sizeof("abc") != 4) return 1;
	if ("abc"[0] != 'a' || "abc"[2] != 'c' || "abc"[3] != 0) return 2;
	p = "a\tb\n\\\"\0x";
	if (p[1] != 9 || p[3] != 10 || p[4] != 92 || p[5] != 34 || p[6] != 0 || p[7] != 'x') return 3;
	if (sizeof("ab\
cd") != 5) return 4;
	p = keep();
	if (p[0] != 'k' || p[3] != 't' || p[4] != 0) return 5;
	if (sizeof("") != 1 || ""[0] != 0) return 6;
	return 0;
}
