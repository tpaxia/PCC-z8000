/* KNR: 7.7.int 7.7.long 7.7.pointer-null 7.7.float 7.7.precedence */
int g;
main()
{
	int i, *p;
	long l;
	double d;
	i = 5;
	if (!(i == 5) || i != 5 || (i == 4) != 0 || (i != 4) != 1) return 1;
	l = 0x10000L;
	if (l == 0 || !(l != 0L) || l == 0x20000L || !(l == 65536L)) return 2;
	p = 0;
	if (p != 0 || !(p == 0)) return 3;
	p = &g;
	if (p == 0 || !(p == &g)) return 4;
	d = 0.25;
	if (d != 0.25 || d == 0.5) return 5;
	if ((1 < 2 == 3 < 4) != 1) return 6;	/* (1<2) == (3<4) */
	return 0;
}
