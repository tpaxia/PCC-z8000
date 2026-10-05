struct bits { unsigned a:3; unsigned b:5; unsigned c:8; };
struct bits b = { 5, 17, 200 };
main()
{
	if (b.a != 5) return 1;
	if (b.b != 17) return 2;
	if (b.c != 200) return 3;
	return 0;
}
