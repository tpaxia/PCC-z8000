/* KNR: 11.1.block-shadow 11.1.param-shadow 11.1.inner-extern 11.1.tag-in-block */
int v = 1;
int w = 100;
shadow(v) { return v + 1; }
peek() { extern int w; return w; }
main()
{
	int r;
	if (v != 1 || shadow(5) != 6) return 1;
	{
		int v;
		v = 2;
		{
			int v;
			v = 3;
			if (v != 3) return 2;
		}
		if (v != 2) return 3;
	}
	if (v != 1) return 4;
	{
		long w;
		w = 70000L;
		if (sizeof w != 4 || peek() != 100) return 5;
	}
	{
		struct local { char a; char b; char c; } s;
		s.c = 'c';
		if (sizeof s != 3 && sizeof s != 4) return 6;
		r = s.c;
	}
	if (r != 'c') return 7;
	return 0;
}
