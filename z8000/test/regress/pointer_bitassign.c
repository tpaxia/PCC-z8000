main()
{
	char a[2];
	unsigned v, bits;
	bits = (unsigned)a;
	v = 0;
	v |= a;
	if (v != bits) return 1;
	v &= a;
	if (v != bits) return 2;
	v ^= a;
	if (v != 0) return 3;
	return 0;
}
