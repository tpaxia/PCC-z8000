main()
{
	int x;
	int *p;
	x = 7;
	p = &x;
	if (sizeof(x++) != 2) return 1;
	if (sizeof(*p++) != 2) return 2;
	if (x != 7 || p != &x) return 3;
	if (sizeof(char) != 1 || sizeof(long) != 4) return 4;
	if (sizeof(float) != 4 || sizeof(double) != 8) return 5;
	return 0;
}
