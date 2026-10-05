main()
{
	int a, b, c;
	a = 2; b = 3; c = 4;
	if (a + b * c != 14) return 1;
	if ((a + b) * c != 20) return 2;
	if ((a << b + 1) != 32) return 3;
	if ((a | b & c) != 2) return 4;
	if ((a == b || b < c && a != c) != 1) return 5;
	a = b = c = 9;
	if (a != 9 || b != 9 || c != 9) return 6;
	return 0;
}
