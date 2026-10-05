/* Double values through pointers, arrays, structures and memory copies. */
union bits { double d; unsigned short w[4]; } a,b;
struct box { int tag; double d; int tail; } s;
double v[3];
set(p,q)
double *p,*q;
{ *p = *q; }
main()
{
	a.d=1.0; b.d=2.0;
	v[0]=a.d; v[1]=b.d;
	set(&v[2],&v[1]);
	s.tag=123; s.tail=456;
	s.d=v[0]+v[2];
	set(&a.d,&s.d);
	if(a.w[0]!=0x4008 || a.w[1] || a.w[2] || a.w[3]) return 1;
	if(s.tag!=123 || s.tail!=456) return 2;
	return 0;
}
