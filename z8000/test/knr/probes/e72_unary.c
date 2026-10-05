/* KNR: 7.2.indirection 7.2.address 7.2.minus 7.2.not 7.2.complement */
int a[3];
main()
{
	int x, *p;
	unsigned u;
	long l;
	x = 7; p = &x;
	if (*p != 7 || *&x != 7 || &*p != p) return 1;
	*p = 9;
	if (x != 9) return 2;
	if (&a[2] - &a[0] != 2) return 3;
	if (-x != -9 || - -x != 9) return 4;
	u = 1;
	if (-u != 65535) return 5;		/* 2**n minus the value */
	l = 70000L;
	if (-l != -70000L) return 6;
	if (!x != 0 || !0 != 1 || !!x != 1) return 7;
	p = 0;
	if (!p != 1) return 8;
	if (~0 != -1 || ~x != -10) return 9;
	if (~l != -70001L) return 10;
	return 0;
}
