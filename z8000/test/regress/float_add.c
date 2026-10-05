/* Check constant byte order, copies, parameters, returns and call spills. */
union bits { double d; unsigned short w[4]; } a,b,c;
double sum(x,y)
double x,y;
{
	double t;
	t=x+y;
	return t;
}
main()
{
	a.d=1.0; b.d=2.0;
	if(a.w[0]!=0x3ff0 || a.w[1] || a.w[2] || a.w[3]) return 1;
	c.d=a.d+b.d;
	if(c.w[0]!=0x4008 || c.w[1] || c.w[2] || c.w[3]) return 2;
	c.d=sum(a.d,b.d)+sum(b.d,a.d);
	if(c.w[0]!=0x4018 || c.w[1] || c.w[2] || c.w[3]) return 3;
	return 0;
}
