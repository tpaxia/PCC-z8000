main()
{
	long x;
	x = 65536L;
	if (!x) return 1;
	x = 1L;
	if (!x) return 2;
	x = 0L;
	if (x) return 3;
	return 0;
}
