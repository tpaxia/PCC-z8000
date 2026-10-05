typedef int a;

int
main()
{
	struct x { int a; } a;

	a.a = 29; if (a.a != 29) abort(); return 0;
}
