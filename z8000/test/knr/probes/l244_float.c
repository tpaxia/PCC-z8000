/* KNR: 2.4.4.forms 2.4.4.is-double */
main()
{
	double d;
	if (sizeof(1.5) != sizeof(double)) return 1;
	d = 1.5;
	if (d != 15e-1 || d != 1.5E0 || d != .15e1 || d != 0.15e+1) return 2;
	if (.5 + .5 != 1.) return 3;
	if (1e2 != 100. || 1E2 != 100.0) return 4;
	if (2. * 4.25 != 8.5) return 5;
	return 0;
}
