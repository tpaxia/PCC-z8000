/* KNR: 7.2.cast-constant
 * A cast applied to a constant converts it like any other operand. */
main()
{
	int i;
	if ((char)300 != 44) return 1;
	i = (int)(char)0x141;
	if (i != 0x41) return 2;
	if ((char)0x1FF != -1) return 3;
	if ((int)70000L != 4464) return 4;
	if ((unsigned)-1 != 65535) return 5;
	return 0;
}
