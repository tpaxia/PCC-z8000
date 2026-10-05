/* KNR: 9.3.if 9.3.else 9.3.dangling-else 9.1.expression 9.2.compound 9.13.null */
main()
{
	int a, b, r;
	a = 1; b = 0; r = 0;
	if (a) r = 1;
	if (r != 1) return 1;
	if (b) r = 2; else r = 3;
	if (r != 3) return 2;
	r = 0;
	if (a)
		if (b) r = 1;
		else r = 2;		/* belongs to the inner if */
	if (r != 2) return 3;
	r = 0;
	if (b)
		if (a) r = 1;
		else r = 2;
	if (r != 0) return 4;
	if (a) { int a; a = 5; r = a; } else ;
	if (r != 5 || a != 1) return 5;
	;;
	if (b) ; else r = 6;
	if (r != 6) return 6;
	a + b;
	if (a == 1) if (b == 1) return 7; else if (b == 0) r = 8; else return 9;
	if (r != 8) return 10;
	return 0;
}
