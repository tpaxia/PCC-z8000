/* KNR: 6.3.float-to-int 6.3.int-to-float 6.3.long */
main()
{
	int i;
	long l;
	double d;
	d = 3.9; i = d;
	if (i != 3) return 1;
	d = -3.9; i = d;
	if (i != -3) return 2;		/* machine dependent: truncates toward zero */
	i = 7; d = i;
	if (d != 7.0) return 3;
	i = -7; d = i;
	if (d != -7.0) return 4;
	d = 100000.7; l = d;
	if (l != 100000L) return 5;
	l = -123456L; d = l;
	if (d != -123456.0) return 6;
	return 0;
}
