/* KNR: 14.2.function-name-as-pointer 14.2.call-through-pointer 14.2.function-argument 14.2.table 14.2.returned */
inc(v) { return v + 1; }
dbl(v) { return v * 2; }
apply(f, v) int (*f)(); { return (*f)(v); }
int (*choose(n))() { return n ? dbl : inc; }
int (*tab[2])() = { inc, dbl };
struct ops { int (*op)(); int arg; } o = { dbl, 21 };
main()
{
	int (*f)();
	f = inc;
	if ((*f)(1) != 2) return 1;
	if (apply(dbl, 4) != 8 || apply(f, 4) != 5) return 2;
	if ((*tab[1])(5) != 10 || (*tab[0])(5) != 6) return 3;
	if ((*choose(1))(6) != 12 || (*choose(0))(6) != 7) return 4;
	if ((*o.op)(o.arg) != 42) return 5;
	if (f != inc || f == dbl) return 6;
	return 0;
}
