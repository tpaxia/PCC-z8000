struct bits { unsigned a:3; unsigned b:5; unsigned c:8; };
main()
{
	struct bits b;
	int old;
	b.a = 5; b.b = 17; b.c = 200;
	old = b.b++;
	if (old != 17 || b.b != 18) return 1;
	b.b += 2;
	if (b.b != 20 || b.a != 5 || b.c != 200) return 2;
	return 0;
}
