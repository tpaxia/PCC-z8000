/* KNR: 7.15.value 7.15.order 7.15.in-argument */
one(v) { return v; }
main()
{
	int a, b, i, j;
	long l;
	b = (a = 1, a + 1);
	if (a != 1 || b != 2) return 1;
	for (i = 0, j = 10; i < j; i++, j--)
		;
	if (i != 5 || j != 5) return 2;
	if (one((a = 3, a * 2)) != 6) return 3;
	if (sizeof(a, l) != sizeof(long)) return 4;
	a = 1, b = 2;
	if (a + b != 3) return 5;
	return 0;
}
