main()
{
	long x, y;
	x = 65538L;
	y = ~x;
	if (y != -65539L) return 1;
	return 0;
}
