/* KNR: 7.5.result-type
 * "The right operand is converted to int; the type of the result is that
 * of the left operand." */
main()
{
	int i;
	long l;
	i = 1; l = 2;
	if (sizeof(i << l) != sizeof(int)) return 1;
	if (sizeof(l << i) != sizeof(long)) return 2;
	if (i << l != 4) return 3;
	return 0;
}
