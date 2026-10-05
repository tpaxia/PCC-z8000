/* KNR: 6.2.float-to-double 6.2.double-to-float */
main()
{
	float f;
	double d;
	f = 1.5; d = f;
	if (d != 1.5) return 1;
	d = 0.1; f = d; 
	if (f == d) return 2;		/* 0.1 is rounded when narrowed */
	d = 16777217.0; f = d; d = f;
	if (d != 16777216.0) return 3;
	d = -2.25; f = d;
	if (f != -2.25) return 4;
	return 0;
}
