main()
{
	long x, old;
	x = 65535L;
	old = x++;
	if (old != 65535L) return 1;
	if (x != 65536L) return 2;
	old = x--;
	if (old != 65536L || x != 65535L) return 3;
	return 0;
}
