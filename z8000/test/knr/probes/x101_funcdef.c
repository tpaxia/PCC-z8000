/* KNR: 10.1.default-int-params 10.1.param-order 10.1.char-param 10.1.float-param 10.1.array-param 10.1.register-param 10.1.return-types 10.1.struct-pointer-param */
struct pt { int x, y; };
mix(a, b, c) long c; char b; { return a + b + (int)c; }
dflt(a, b) { return a - b; }
chr(c) char c; { return c; }
fpar(f) float f; { return sizeof f; }	/* float parameters are adjusted to double */
arr(v) int v[]; { return sizeof v == sizeof(int *) ? v[1] : -1; }
reg(n, p) register n; register char *p; { while (n--) p++; return *p; }
char *last(s) char *s; { while (s[1]) s++; return s; }
double avg(a, b) double a, b; { return (a + b) / 2; }
long widen(v) { return v; }
sum(p) struct pt *p; { return p->x + p->y; }
main()
{
	int a[3];
	struct pt q;
	if (mix(1, 2, 3L) != 6 || dflt(9, 4) != 5) return 1;
	if (chr('z') != 'z' || chr(0x141) != 0x41) return 2;
	if (fpar(1.0) != sizeof(double)) return 3;
	a[1] = 42;
	if (arr(a) != 42) return 4;
	if (reg(2, "abcd") != 'c') return 5;
	if (*last("hello") != 'o') return 6;
	if (avg(1.0, 2.0) != 1.5) return 7;
	if (widen(-2) != -2L) return 8;
	q.x = 3; q.y = 4;
	if (sum(&q) != 7) return 9;
	return 0;
}
