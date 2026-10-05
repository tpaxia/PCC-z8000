/* KNR: 8.2.short-long-unsigned 8.2.long-float 8.2.default-int 4.fundamental-types */
static deflt = 5;
short int si; long int li; unsigned int ui;
short s; long l; unsigned u;
long float lf;
char c; int i; float f; double d;
bare() { register n; n = 6; return n; }
main()
{
	if (sizeof si != 2 || sizeof li != 4 || sizeof ui != 2) return 1;
	if (sizeof s != 2 || sizeof l != 4 || sizeof u != 2) return 2;
	if (sizeof lf != sizeof(double)) return 3;	/* long float is double */
	if (sizeof c != 1 || sizeof i != 2 || sizeof f != 4 || sizeof d != 8) return 4;
	if (sizeof deflt != sizeof(int) || deflt != 5 || bare() != 6) return 5;
	s = -2; u = 40000; l = -70000L;
	if (s != -2 || u != 40000 || l != -70000L) return 6;
	return 0;
}
