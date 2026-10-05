/* KNR: 7.2.cast 7.2.sizeof-expr 7.2.sizeof-type */
struct s { char c; int i; long l; };
main()
{
	int i, a[5];
	double d;
	i = 300;
	if ((char)i != 44) return 1;
	d = 3.7;
	if ((int)d != 3) return 2;
	if ((long)-1 != -1L) return 3;
	if ((unsigned)-1 != 65535) return 4;
	i = 1;
	if ((double)i / 2 != 0.5) return 5;
	if (sizeof(char) != 1 || sizeof(short) != 2 || sizeof(int) != 2 || sizeof(long) != 4) return 6;
	if (sizeof(float) != 4 || sizeof(double) != 8 || sizeof(int *) != 2) return 7;
	if (sizeof a != 10 || sizeof(a) / sizeof(a[0]) != 5) return 8;
	if (sizeof(struct s) != 8) return 9;
	i = 0;
	if (sizeof(i++) != 2 || i != 0) return 10;	/* operand is not evaluated */
	if (sizeof i + 1 != 3) return 11;
	return 0;
}
