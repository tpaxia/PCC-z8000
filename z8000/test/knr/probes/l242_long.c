/* KNR: 2.4.2.suffix */
main()
{
	long a;
	if (sizeof(1L) != 4 || sizeof(1l) != 4 || sizeof(0x1L) != 4 || sizeof(01L) != 4) return 1;
	a = 100000L;
	if (a / 1000 != 100) return 2;
	if (2147483647L - 2147483646L != 1) return 3;
	if (0x7FFFFFFFL != 2147483647L) return 4;
	return 0;
}
