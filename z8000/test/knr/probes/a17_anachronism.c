/* KNR: 17.old-assignment-ops 17.old-initializer
 * "Earlier versions of C used the form =op instead of op=" and allowed
 * an initializer without the equals sign. */
int old 5;
main()
{
	int x;
	x = 10;
	x =+ 2;
	if (x != 12) return 1;
	x =* 3;
	if (x != 36) return 2;
	if (old != 5) return 3;
	return 0;
}
