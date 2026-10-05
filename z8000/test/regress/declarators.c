typedef int (*fn)();
int add(a, b) int a, b; { return a + b; }
int sub(a, b) int a, b; { return a - b; }
fn funcs[2] = { add, sub };
int a[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
main()
{
	int (*p)[3];
	fn f;
	p = a;
	if (p[1][2] != 6) return 1;
	f = funcs[0];
	if ((*f)(7, 3) != 10) return 2;
	if ((*funcs[1])(7, 3) != 4) return 3;
	if (sizeof(a) != 12 || sizeof(*p) != 6) return 4;
	return 0;
}
