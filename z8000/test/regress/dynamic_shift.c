main()
{
	register int n;
	int w;
	unsigned u;
	long l;
	n = 4;
	w = -256;
	w = w >> n;
	if (w != -16) return 1;
	if (n != 4) return 2;
	u = 0x8000;
	u = u >> n;
	if (u != 2048 || n != 4) return 3;
	l = 1048576L;
	l = l >> n;
	if (l != 65536L || n != 4) return 4;
	return 0;
}
