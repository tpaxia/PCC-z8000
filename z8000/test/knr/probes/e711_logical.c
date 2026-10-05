/* KNR: 7.11.and 7.12.or 7.11.short-circuit 7.11.result */
int calls;
hit(v) { calls++; return v; }
main()
{
	int *p;
	long l;
	double d;
	if ((2 && 3) != 1 || (2 && 0) != 0 || (0 || 0) != 0 || (0 || 7) != 1) return 1;
	calls = 0;
	if (hit(0) && hit(1)) return 2;
	if (calls != 1) return 3;
	calls = 0;
	if (!(hit(1) || hit(1))) return 4;
	if (calls != 1) return 5;
	calls = 0;
	if ((hit(1) && hit(0) || hit(1)) != 1 || calls != 3) return 6;
	if ((1 || 0 && 0) != 1) return 7;	/* && binds tighter */
	p = 0; l = 0x10000L; d = 0.5;
	if (p && 1) return 8;
	if (!(l && d)) return 9;
	return 0;
}
