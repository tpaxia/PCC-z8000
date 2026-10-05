/* KNR: 9.4.while 9.5.do 9.6.for 9.6.for-omitted 9.8.break 9.9.continue */
main()
{
	int i, j, n;
	n = 0; i = 0;
	while (i < 5) { n += i; i++; }
	if (n != 10) return 1;
	while (0) return 2;
	n = 0; i = 10;
	do { n++; } while (i < 5);
	if (n != 1) return 3;
	n = 0;
	for (i = 0; i < 4; i++) n += i;
	if (n != 6 || i != 4) return 4;
	i = 0;
	for (;;) { if (++i == 7) break; }
	if (i != 7) return 5;
	i = 0; n = 0;
	for (; i < 3;) { i++; n++; }
	if (n != 3) return 6;
	n = 0;
	for (i = 0; i < 10; i++) { if (i % 2) continue; n += i; }
	if (n != 20) return 7;
	n = 0; i = 0;
	while (i < 10) { i++; if (i < 8) continue; n++; }
	if (n != 3) return 8;
	n = 0; i = 0;
	do { i++; if (i == 2) continue; n++; } while (i < 4);
	if (n != 3) return 9;
	n = 0;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 5; j++) { if (j == 2) break; n++; }
	if (n != 6) return 10;
	i = 5;
	do i--; while (i);
	if (i != 0) return 11;
	return 0;
}
