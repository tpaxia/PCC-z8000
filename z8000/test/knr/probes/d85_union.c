/* KNR: 8.5.union 8.5.union-size 8.5.union-in-struct */
union u { int i; char c[2]; long l; };
struct tagged { int kind; union { int n; char *s; long l; } v; };
main()
{
	union u x;
	struct tagged t;
	if (sizeof(union u) != 4) return 1;
	x.l = 0;
	x.i = 0x1234;
	/* machine dependent: the Z8000 stores the high byte first */
	if (x.c[0] != 0x12 || x.c[1] != 0x34) return 2;
	x.l = 0x01020304L;
	if (x.c[0] != 1 || x.c[1] != 2 || x.i != 0x0102) return 3;
	t.kind = 1; t.v.n = 77;
	if (t.v.n != 77) return 4;
	t.v.s = "q";
	if (*t.v.s != 'q') return 5;
	if (sizeof t != 6) return 6;
	if ((char *)&x.i != (char *)&x.l || (char *)&x.c[0] != (char *)&x) return 7;
	return 0;
}
