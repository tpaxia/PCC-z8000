/* KNR: 2.4.1.decimal 2.4.1.octal 2.4.1.hex 2.4.1.auto-long */
main()
{
	if (012 != 10 || 0777 != 511 || 00 != 0) return 1;
	if (0x1F != 31 || 0X1f != 31 || 0xabc != 2748) return 2;
	if (32767 + 0 != 32767) return 3;
	if (sizeof(32767) != sizeof(int)) return 4;
	/* a decimal constant too big for an int is long */
	if (sizeof(32768) != sizeof(long)) return 5;
	/* an octal or hex constant is long only beyond the largest unsigned */
	if (sizeof(0177777) != sizeof(int)) return 6;
	if (sizeof(0xFFFF) != sizeof(int)) return 7;
	if (sizeof(0200000) != sizeof(long)) return 8;
	if (sizeof(0x10000) != sizeof(long)) return 9;
	if (0x10000 != 65536L) return 10;
	if (100000 / 10 != 10000) return 11;
	return 0;
}
