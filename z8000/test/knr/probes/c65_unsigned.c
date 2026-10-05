/* KNR: 6.5.modulo 6.5.mixed-with-int 6.4.pointer-integer */
int arr[4];
main()
{
	unsigned u;
	int i;
	int *p;
	u = -1;
	if (u != 65535) return 1;
	if (u + 1 != 0) return 2;
	i = -1;
	if (u != i) return 3;		/* i is converted to unsigned */
	u = 1; i = -2;
	if (!(u + i > 0)) return 4;	/* 65535, not -1 */
	u = 65535;
	if (u / 2 != 32767 || u >> 1 != 32767) return 5;
	if (u < 1) return 6;
	p = arr; i = 2;
	if (p + i != &arr[2] || i + p != &arr[2]) return 7;
	return 0;
}
