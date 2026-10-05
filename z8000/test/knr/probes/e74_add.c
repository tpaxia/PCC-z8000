/* KNR: 7.4.int 7.4.pointer-plus-int 7.4.pointer-minus-pointer 7.4.long 7.4.float */
struct s { int a; char b; long c; } sa[4];
long la[4];
main()
{
	int i, *p, ia[4];
	long l, *lp;
	struct s *sp;
	double d;
	i = 30000;
	if (i + 2767 != 32767 || i - 30001 != -1) return 1;
	p = ia;
	if (p + 2 != &ia[2] || 2 + p != &ia[2] || (p + 3) - 1 != &ia[2]) return 2;
	lp = la; sp = sa;
	if ((char *)(lp + 1) - (char *)lp != 4) return 3;
	if ((char *)(sp + 1) - (char *)sp != sizeof(struct s)) return 4;
	if (&ia[3] - &ia[0] != 3 || &la[3] - &la[1] != 2 || &sa[3] - sa != 3) return 5;
	if (&ia[0] - &ia[2] != -2) return 6;
	l = 65535L;
	if (l + 1 != 65536L || l - 65536L != -1) return 7;
	d = 0.5;
	if (d + 0.25 != 0.75 || d - 1 != -0.5) return 8;
	if (10 - 4 - 3 != 3) return 9;
	return 0;
}
