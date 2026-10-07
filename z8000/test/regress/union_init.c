union word { int i; long l; };
static union word a = { 37 };
static union word b = 0;
static union word list[2] = { { 11 }, { 23 } };
struct outer { union word w; int tail; };
static struct outer c = { { 41 }, 53 };
main()
{
	if (a.i != 37 || b.i != 0) return 1;
	if (list[0].i != 11 || list[1].i != 23) return 2;
	if (c.w.i != 41 || c.tail != 53) return 3;
	return 0;
}
