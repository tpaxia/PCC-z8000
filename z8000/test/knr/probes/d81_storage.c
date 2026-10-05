/* KNR: 8.1.auto 8.1.static-local 8.1.extern 8.1.register 8.1.static-file 4.storage-classes */
int shared = 3;
static int hidden = 4;
static twice(v) { return v + v; }
counter() { static int n = 0; return ++n; }
fresh() { auto int n; n = 0; return ++n; }
main()
{
	register int r;
	register char *rp;
	extern int shared;
	auto int a;
	counter(); counter();
	if (counter() != 3) return 1;
	fresh();
	if (fresh() != 1) return 2;
	if (shared != 3 || hidden != 4 || twice(hidden) != 8) return 3;
	a = 0;
	for (r = 0; r < 10; r++) a += r;
	if (a != 45) return 4;
	rp = "xyz";
	if (*++rp != 'y') return 5;
	return 0;
}
