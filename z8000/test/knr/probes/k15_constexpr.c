/* KNR: 15.array-bound 15.initializer 15.sizeof 15.case */
int a[2 * 3 + 1];
int b[sizeof(long) * 2];
int c[(1 << 3) - 2];
int d['b' - 'a' + 1];
int v1 = 7 / 2 + 7 % 4;
int v2 = ~0 & 0xF0 | 1;
int v3 = -(3 - 5);
int v4 = (10 > 3) + (2 == 2) + !5;
long v6 = 1000L * 1000L;
int v7 = sizeof(int) * 3;
main()
{
	if (sizeof a != 14 || sizeof b != 16 || sizeof c != 12 || sizeof d != 4) return 1;
	if (v1 != 6 || v2 != 0xF1 || v3 != 2 || v4 != 2) return 2;
	if (v6 != 1000000L || v7 != 6) return 3;
	switch (9) { case 3 * 3: break; default: return 4; }
	switch (4) { case sizeof(long): break; default: return 5; }
	switch ('c') { case 'a' + 2: break; default: return 6; }
	return 0;
}
