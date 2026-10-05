/* KNR: 2.4.1.octal-8-9
 * "The digits 8 and 9 have octal value 10 and 11 respectively." */
main()
{
	if (08 != 8) return 1;
	if (09 != 9) return 2;
	if (019 != 17) return 3;
	return 0;
}
