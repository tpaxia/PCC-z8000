/* Storing a double through a pointer in a function that has register
 * variables: the address needs one address register, not a four-word group. */
double d, arr[4];
double half() { return 0.5; }
struct s { int a; double v; } recs[3], *sp;
direct(ptr) double *ptr; { register c; c = 1; *ptr = d; return c; }
viacall(ptr) int *ptr; { register c; c = 2; **(double **)ptr = half(); return c; }
indexed(ptr, i) double *ptr; register i; { register c, e; c = 1; e = 3; ptr[i + c] = d * e; return c + e; }
member() { register c, e; c = 1; e = 2; sp->v = d; (sp + c)->v = d + e; return c + e; }
main()
{
	double x, *px;
	d = 2.5; px = &x;
	if (direct(&x) != 1 || x != 2.5) return 1;
	if (viacall((int *)&px) != 2 || x != 0.5) return 2;
	if (indexed(arr, 1) != 4 || arr[2] != 7.5 || arr[1] != 0.0) return 3;
	sp = recs;
	if (member() != 3 || recs[0].v != 2.5 || recs[1].v != 4.5 || recs[0].a != 0) return 4;
	return 0;
}
