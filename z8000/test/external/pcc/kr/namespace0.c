typedef int a;
typedef int b;

int
main()
{
	enum a { a = 1, b = a + 2, c = a + b + 3 };

	if (a != 1 || b != 3 || c != 7) abort(); return 0;
}
