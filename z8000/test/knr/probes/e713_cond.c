/* KNR: 7.13.select 7.13.one-evaluated 7.13.right-assoc 7.13.conversion 7.13.pointer */
int calls, g;
hit(v) { calls++; return v; }
main()
{
	int i, *p;
	long l;
	i = 1;
	if ((i ? 10 : 20) != 10 || (!i ? 10 : 20) != 20) return 1;
	calls = 0;
	i = 1 ? hit(5) : hit(6);
	if (i != 5 || calls != 1) return 2;
	i = 2;
	if ((i == 1 ? 1 : i == 2 ? 2 : 3) != 2) return 3;
	if (sizeof(i ? 1 : 1L) != sizeof(long)) return 4;
	if ((i ? 1 : 2.5) != 1.0 || sizeof(i ? 1 : 2.5) != sizeof(double)) return 5;
	l = i ? 70000L : 1;
	if (l != 70000L) return 6;
	p = i ? &g : 0;
	if (p != &g) return 7;
	p = i == 0 ? &g : 0;
	if (p != 0) return 8;
	return 0;
}
