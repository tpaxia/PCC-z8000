/* A typedef name must still be a type when it follows another type with no
 * identifier in between: consecutive casts, sizeof then a cast, and a
 * declaration after a tag-only struct or enum declaration. */
typedef int item;
typedef char *text;
struct only { int a; int b; };
item after_struct;
enum tagonly { first = 3, second };
item after_enum;
text name = "ab";
struct only *make();
item twice(v) item v; { return (item)(item)v + (int)(item)1; }
main()
{
	item x;
	long big;
	big = 70000L;
	x = (item)(unsigned)(item)big;
	if (x != 4464) return 1;
	if (twice(20) != 21) return 2;
	if (sizeof(item) + (item)3 != 5) return 3;
	after_struct = (item)sizeof(struct only);
	after_enum = (item)second;
	if (after_struct != 4 || after_enum != 4) return 4;
	if (*(text)(char *)name != 'a') return 5;
	{ int item; item = 13; if (item != 13) return 6; }
	{ item y; y = (item)(item)17; if (y != 17) return 7; }
	return 0;
}
