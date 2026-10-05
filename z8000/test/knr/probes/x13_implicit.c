/* KNR: 13.undeclared-function 13.class-only */
static sfile = 2;
main()
{
	register r;
	static s;
	auto a;
	r = 1; a = 3; s = sfile;
	if (r + s + a != 6) return 1;
	if (unseen(4) != 16) return 2;
	if (sizeof(unseen(1)) != sizeof(int)) return 3;
	return 0;
}
unseen(v) { return v * v; }
