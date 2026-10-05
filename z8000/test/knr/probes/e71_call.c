/* KNR: 7.1.call-implicit-int 7.1.call-char-promoted 7.1.call-float-promoted 7.1.call-many-args 7.1.call-long 13.undeclared-function */
long big();
double half();
main()
{
	char c;
	float f;
	c = 'x'; f = 3.0;
	if (later(4) != 8) return 1;		/* used before any declaration */
	if (wide(c) != sizeof(int)) return 2;
	if (half(f) != 1.5) return 3;
	if (sum8(1, 2, 3, 4, 5, 6, 7, 8) != 36) return 4;
	if (big(70000L, 3) != 210000L) return 5;
	if (sum8(later(1), 2, later(3), 4, 5, 6, 7, 8) != 40) return 6;
	return 0;
}
later(n) { return n + n; }
wide(ch) char ch; { return sizeof(ch) == 1 ? sizeof(int) : 0; }
double half(d) double d; { return d / 2; }
sum8(a, b, c, d, e, f, g, h) { return a + b + c + d + e + f + g + h; }
long big(l, n) long l; { return l * n; }
