/* KNR: 7.6.result-wide-operands
 * A comparison yields an int whatever the operand types, so it can be
 * passed where an int is expected. */
take(a, b) { return a * 10 + b; }
main()
{
	long l, m;
	double d;
	l = 1; m = 2; d = 0.5;
	if (sizeof(l < m) != sizeof(int)) return 1;
	if (sizeof(l == m) != sizeof(int)) return 2;
	if (sizeof(d < 1.0) != sizeof(int)) return 3;
	if (take(l < m, 7) != 17) return 4;
	if (take(l == m, 7) != 7) return 5;
	return 0;
}
