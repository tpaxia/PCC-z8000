/* pcc_structret.c -- struct return from function
 * Adapted from pcc-tests/tests/c/codegen/struct2.c */

struct str {
	int i;
};

struct str init(v)
int v;
{
	struct str s;
	s.i = v;
	return s;
}

value(s)
struct str s;
{
	return s.i;
}

main()
{
	struct str r;
	struct str (*fp)();

	r = init(10);
	if (r.i != 10) return 1;

	r = init(99);
	if (r.i != 99) return 2;
	if (init(37).i != 37) return 3;
	fp = init;
	if ((*fp)(81).i != 81) return 4;
	if (value(init(53)) != 53) return 5;

	return 0;
}
