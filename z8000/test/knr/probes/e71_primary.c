/* KNR: 7.1.subscript 7.1.call-by-value 7.1.recursion 7.1.member 7.1.arrow 7.1.array-to-pointer 7.1.parenthesized */
struct pt { int x; int y; } g;
int a[4];
change(v) int v; { v = 99; return v; }
fib(n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }
first(p) int *p; { return *p; }
main()
{
	struct pt *pp;
	int v;
	a[0] = 10; a[1] = 11; a[2] = 12; a[3] = 13;
	if (a[2] != 12 || 2[a] != 12 || (a)[3] != 13 || *(a + 1) != 11) return 1;
	v = 5;
	if (change(v) != 99 || v != 5) return 2;
	if (fib(10) != 55) return 3;
	g.x = 3; g.y = 4; pp = &g;
	if (g.x != 3 || pp->y != 4 || (*pp).x != 3 || (&g)->y != 4) return 4;
	if (first(a) != 10 || first(&a[3]) != 13) return 5;
	if (((v)) != 5 || (v + 1) * 2 != 12) return 6;
	return 0;
}
