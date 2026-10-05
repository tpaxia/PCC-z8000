/* KNR: 7.8.and 7.9.xor 7.10.or 7.8.long 7.8.precedence */
main()
{
	int a, b;
	unsigned u;
	long l, m;
	a = 0x0FF0; b = 0x3C3C;
	if ((a & b) != 0x0C30 || (a | b) != 0x3FFC || (a ^ b) != 0x33CC) return 1;
	u = 0xF0F0;
	if ((u & 0x0FF0) != 0x00F0 || (u | 0x000F) != 0xF0FF || (u ^ 0xFFFF) != 0x0F0F) return 2;
	l = 0x12345678L; m = 0xFFFF0000L;
	if ((l & m) != 0x12340000L || (l | m) != 0xFFFF5678L || (l ^ m) != 0xEDCB5678L) return 3;
	if ((l & 0xFF) != 0x78) return 4;
	a = 6; b = 3;
	if ((a & b == 3) != 0) return 5;	/* == binds tighter than & */
	if ((1 | 2 ^ 3 & 4) != 3) return 6;	/* & then ^ then | */
	return 0;
}
