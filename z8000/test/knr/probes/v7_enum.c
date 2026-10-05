/* KNR: V7.enum-values V7.enum-explicit V7.enum-variable V7.enum-switch */
enum color { red, green, blue };
enum level { low = 10, mid, high = 20, top };
struct paint { enum color c; int n; };
name(c) enum color c;
{
	switch (c) {
	case red: return 'r';
	case green: return 'g';
	case blue: return 'b';
	}
	return '?';
}
main()
{
	enum color c;
	enum level v;
	struct paint p;
	if (red != 0 || green != 1 || blue != 2) return 1;
	if (low != 10 || mid != 11 || high != 20 || top != 21) return 2;
	c = green; v = top;
	if (c != green || v != top) return 3;
	if (name(blue) != 'b' || name(c) != 'g') return 4;
	p.c = blue; p.n = 1;
	if (p.c != blue || sizeof(enum color) != sizeof(int)) return 5;
	return 0;
}
