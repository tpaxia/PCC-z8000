/* KNR: 6.6.char-short-to-int 6.6.float-to-double 6.6.long 6.6.unsigned */
main()
{
	char c, d;
	short s;
	int i;
	unsigned u;
	long l;
	float f;
	c = 100; d = 100;
	if (c + d != 200) return 1;
	if (sizeof(c + d) != sizeof(int)) return 2;
	s = 3;
	if (sizeof(s * s) != sizeof(int)) return 3;
	f = 1;
	if (sizeof(f + f) != sizeof(double)) return 4;
	i = 1000;
	l = i * 100L;
	if (l != 100000L) return 5;
	if (sizeof(i + l) != sizeof(long)) return 6;
	i = -1;
	if (!(i < 1L)) return 7;
	u = 65535; l = u;
	if (l != 65535L) return 8;	/* unsigned widens without sign */
	if (sizeof(u + i) != sizeof(unsigned)) return 9;
	if (sizeof(i + 1.0) != sizeof(double)) return 10;
	return 0;
}
