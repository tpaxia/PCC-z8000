/* KNR: 7.3.int 7.3.unsigned 7.3.long 7.3.float */
main()
{
	int a, b;
	unsigned u;
	long l;
	double d;
	a = 7; b = 6;
	if (a * b != 42 || a / 2 != 3 || a % 2 != 1) return 1;
	a = -7; b = 2;
	if ((a / b) * b + a % b != a) return 2;
	if (a / b != -3 || a % b != -1) return 3;	/* machine dependent: truncation */
	u = 60000;
	if (u / 7 != 8571 || u % 7 != 3) return 4;
	if (u * 2 != 54464) return 5;
	l = 100000L;
	if (l * 3 != 300000L || l / 7 != 14285 || l % 7 != 5) return 6;
	l = -100000L;
	if (l / 7 != -14285 || l % 7 != -5) return 7;
	d = 1.5;
	if (d * 2 != 3.0 || 7.0 / 2 != 3.5 || d * d != 2.25) return 8;
	if (2 * 3 * 4 != 24 || 24 / 4 / 2 != 3 || 2 + 3 * 4 != 14) return 9;
	return 0;
}
