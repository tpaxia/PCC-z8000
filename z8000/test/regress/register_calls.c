long wide(x) int x; { return x + 65536L; }
f(x) int x; { register int a; a = x; return a * 3; }
main()
{
	register int a, b, c, d;
	long x;
	a = 11; b = 13; c = 17; d = 19;
	x = wide(f(a) + f(b));
	if (x != 65608L) return 1;
	if (a != 11 || b != 13 || c != 17 || d != 19) return 2;
	return 0;
}
