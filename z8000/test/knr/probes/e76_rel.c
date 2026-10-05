/* KNR: 7.6.int 7.6.unsigned 7.6.pointer 7.6.long 7.6.float 7.6.result 7.6.grouping */
int a[4];
main()
{
	int i, j;
	unsigned u;
	long l, m;
	double d;
	i = -1; j = 1;
	if (!(i < j) || i > j || !(i <= j) || i >= j || !(j >= j) || !(j <= j)) return 1;
	u = 65535;
	if (!(u > 1) || u < 1) return 2;
	if (!(&a[1] < &a[2]) || &a[3] <= &a[0] || !(&a[1] >= &a[1])) return 3;
	l = -1; m = 1;
	if (!(l < m) || l > m) return 4;
	l = 100000L; m = 65535L;
	if (!(l > m) || l <= m) return 5;
	l = 0x10000L; m = 0x0FFFFL;
	if (l < m) return 6;
	d = 0.5;
	if (!(d < 1.0) || d > 0.75 || !(d >= 0.5)) return 7;
	if ((3 < 4) != 1 || (4 < 3) != 0 || sizeof(i < j) != sizeof(int)) return 8;
	if ((3 < 2 < 1) != 1) return 9;		/* (3<2)<1 */
	return 0;
}
