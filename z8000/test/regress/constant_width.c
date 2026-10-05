main()
{
	unsigned u;
	unsigned long l;
	u = 65535L;
	if ((unsigned)(u + 1) != (unsigned)(65535L + 1)) return 1;
	if (((unsigned)65535L >> 1) != 32767) return 2;
	l = (unsigned long)-1L;
	if ((l >> 1) != (unsigned long)2147483647L) return 3;
	if (((unsigned long)-1L >> 1) != (l >> 1)) return 4;
	return 0;
}
