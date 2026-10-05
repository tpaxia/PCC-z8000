/* KNR: 8.8.scalar 8.8.pointer 8.8.struct 8.8.array 8.8.function-pointer 8.8.block */
typedef int MILES, *MP;
typedef struct { double re, im; } complex;
typedef int vec[3];
typedef int (*handler)();
typedef char *string;
nine() { return 9; }
main()
{
	MILES m;
	MP p;
	complex z, *zp;
	vec v;
	handler h;
	string s;
	m = 5; p = &m;
	if (*p != 5 || sizeof(MILES) != sizeof(int)) return 1;
	z.re = 1.5; z.im = -1.5; zp = &z;
	if (zp->re + zp->im != 0.0 || sizeof(complex) != 16) return 2;
	v[2] = 8;
	if (sizeof v != 6 || sizeof(vec) != 6 || v[2] != 8) return 3;
	h = nine;
	if ((*h)() != 9) return 4;
	s = "ok";
	if (s[1] != 'k') return 5;
	{
		typedef long big;
		big b;
		b = 70000L;
		if (sizeof b != 4 || b != 70000L) return 6;
	}
	return 0;
}
