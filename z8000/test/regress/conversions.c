main()
{
	long x, l;
	int i, w;
	unsigned u;
	i = -123; u = 60000;
	l = 305419896L;
	x = i;
	if (x != -123L) return 1;
	x = u;
	if (x != 60000L) return 2;
	w = l;
	if (w != 22136) return 5;
	return 0;
}
