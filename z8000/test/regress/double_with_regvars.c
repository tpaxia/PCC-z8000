/* Doubles in functions that also have register variables. A double needs four
 * word registers, so with register variables in use only one such group is
 * free; plain assignments, conditions and sums must still compile and run. */
double dv, dw, vals[4];
int iv;
long lv, lw;
double half() { return 0.5; }
twice(a) { return a + a; }
copy(a) register a; { dv = dw; return a; }
fromcall(a) register a; { dv = half(); return a; }
fromint(a) register a; { dv = a; return a; }
double sum(p, n) register double *p; register n;
{
	double s;
	s = 0;
	while (n--) s += *p++;
	return s;
}
cond(a, b, c, d) register a; { iv = a * b + (dv ? c : d); return iv; }
condcall(a, c, d) register a; { iv = twice(a) + (dv ? c : d); return iv; }
condlong(a, c, d) register a; { lv = lw * a + (dv ? c : d); return 0; }
many(a, b, c) register a, b, c;
{
	register d;
	dv = dw;
	d = dv < vals[3];
	return d + a + b + c;
}
main()
{
	dw = 2.5;
	if (copy(3) != 3 || dv != 2.5) return 1;
	if (fromcall(4) != 4 || dv != 0.5) return 2;
	if (fromint(-7) != -7 || dv != -7.0) return 3;
	vals[0] = 1.5; vals[1] = 2.25; vals[2] = -0.75; vals[3] = 4.0;
	if (sum(vals, 4) != 7.0) return 4;
	dv = 1.0;
	if (cond(3, 4, 5, 6) != 17) return 5;
	dv = 0.0;
	if (cond(3, 4, 5, 6) != 18) return 6;
	dv = -0.0;
	if (cond(3, 4, 5, 6) != 18) return 7;
	dv = 2.0;
	if (condcall(10, 1, 2) != 21) return 8;
	lw = 70000L; dv = 0.0;
	condlong(3, 1, 2);
	if (lv != 210002L) return 9;
	dw = 3.0;
	if (many(1, 2, 3) != 7 || dv != 3.0) return 10;
	return 0;
}
