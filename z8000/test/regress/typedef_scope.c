typedef int item;
enum color { red = 2, green, blue = 7 };
struct pair { item x; enum color y; };
main()
{
	item x;
	struct pair p;
	x = 11;
	p.x = x;
	p.y = green;
	{ int item; item = 13; if (item != 13) return 1; }
	{ item y; y = 17; if (y != 17) return 2; }
	if (p.x != 11 || p.y != 3 || blue != 7) return 3;
	return 0;
}
