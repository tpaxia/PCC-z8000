/* KNR: 7.5.left 7.5.right-signed 7.5.right-unsigned 7.5.long 7.5.variable-count */
main()
{
	int i, n;
	unsigned u;
	long l;
	i = 1;
	if (i << 4 != 16 || i << 0 != 1) return 1;
	i = -16;
	if (i >> 2 != -4) return 2;		/* machine dependent: arithmetic */
	u = 0x8000;
	if (u >> 15 != 1 || u >> 4 != 0x0800) return 3;
	l = 1;
	if (l << 20 != 1048576L) return 4;
	l = 0x12345678L;
	if (l >> 16 != 0x1234 || l >> 4 != 0x01234567L) return 5;
	n = 3; i = 5;
	if (i << n != 40 || 40 >> n != 5) return 6;
	n = 17; l = 1;
	if (l << n != 131072L) return 7;
	if (1 << 2 + 1 != 8) return 8;		/* + binds tighter */
	return 0;
}
