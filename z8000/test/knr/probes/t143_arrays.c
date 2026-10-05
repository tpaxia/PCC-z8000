/* KNR: 14.3.subscript-identity 14.3.row-major 14.3.multidimensional 14.3.negative-index */
int m[3][4];
char c3[2][3][2];
main()
{
	int i, j, *p, a[5];
	for (i = 0; i < 5; i++) a[i] = i * 10;
	if (a[3] != *(a + 3) || 3[a] != a[3] || *(&a[1] + 2) != 30) return 1;
	p = &a[4];
	if (p[-1] != 30 || p[-4] != 0 || *(p - 2) != 20) return 2;
	for (i = 0; i < 3; i++) for (j = 0; j < 4; j++) m[i][j] = i * 4 + j;
	p = &m[0][0];
	for (i = 0; i < 12; i++) if (p[i] != i) return 3;
	if (&m[1][0] - &m[0][0] != 4 || *m[2] != 8 || *(*(m + 1) + 2) != 6) return 4;
	if (sizeof m != 24 || sizeof m[0] != 8 || sizeof m[0][0] != 2) return 5;
	c3[1][2][1] = 'q';
	if (*((char *)c3 + 11) != 'q' || sizeof c3 != 12) return 6;
	if ((char *)(m + 1) - (char *)m != 8) return 7;
	return 0;
}
