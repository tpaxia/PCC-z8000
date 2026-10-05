/* KNR: 15.conditional
 * A constant expression may use the ?: operator. */
int e[1 ? 4 : 9];
int v = 0 ? 2 : 3;
main()
{
	if (sizeof e != 8 || v != 3) return 1;
	switch (5) { case 1 ? 5 : 6: break; default: return 2; }
	return 0;
}
