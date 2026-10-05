/* KNR: 2.2.runtime-names
 * No identifier is reserved beyond the keywords, so a program may use any
 * name, including ones the compiler's own support routines happen to have. */
int flt = 3;
int fadd = 4;
long lmul = 5;
ldiv(v) { return v + 1; }
main()
{
	double d;
	long l;
	d = 1.5; l = 70000L;
	if (d + d != 3.0 || !(d < 2.0)) return 1;	/* uses the floating runtime */
	if (l * 3 != 210000L || l / 7 != 10000) return 2;	/* uses the long runtime */
	if (flt != 3 || fadd != 4 || lmul != 5 || ldiv(1) != 2) return 3;
	return 0;
}
