struct small { char a[3]; };
struct small g;
main()
{
	struct small a, b;
	a.a[0] = 7; a.a[1] = 13; a.a[2] = 29;
	b = a; g = b;
	if (g.a[0] != 7 || g.a[1] != 13 || g.a[2] != 29) return 1;
	return 0;
}
