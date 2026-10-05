/* Exercise equal high words, differing signs, and unsigned high-bit values. */
main()
{
	long a, b;
	unsigned long u, v;
	a = 65535L; b = 65536L;
	if (!(a < b) || a >= b || a == b) return 1;
	a = -65535L; b = -65536L;
	if (!(a > b) || a <= b) return 2;
	a = -1L; b = 65535L;
	if (!(a < b) || a == b) return 3;
	u = (unsigned long)-1L; v = 2147483647L;
	if (!(u > v) || u <= v) return 4;
	return 0;
}
