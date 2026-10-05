/* KNR: 9.11.goto 9.12.label 9.10.return 11.1.label-scope */
long wide() { return 7; }
double real() { return 2; }
none() { return; }
early(v) { if (v) return 1; return 2; }
main()
{
	int i, j, n;
	n = 0; i = 0;
again:
	n += i;
	if (++i < 5) goto again;
	if (n != 10) return 1;
	goto skip;
	return 2;
skip:	;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (i * j == 4) goto out;
out:
	if (i != 2 || j != 2) return 3;
	{
		goto inner;
		n = 99;
	inner:	n++;
	}
	if (n != 11) return 4;
	if (wide() != 7L || sizeof(wide()) != 4) return 5;
	if (real() != 2.0) return 6;
	none();
	if (early(1) != 1 || early(0) != 2) return 7;
	return 0;
}
