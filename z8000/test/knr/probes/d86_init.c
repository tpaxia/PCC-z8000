/* KNR: 8.6.scalar 8.6.array 8.6.partial 8.6.size-from-init 8.6.string 8.6.struct 8.6.elided-braces 8.6.pointer 8.6.auto-expression 8.6.default-zero 8.6.long-float */
int s1 = 5, s2 = -3;
int full[3] = { 1, 2, 3 };
int part[5] = { 7, 8 };
int sized[] = { 4, 5, 6, 7 };
char str[] = "abc";
char fixed[6] = "hi";
struct p { int x; char c; long l; };
struct p one = { 1, 'q', 70000L };
struct p many[2] = { 1, 'a', 10L, 2, 'b', 20L };
struct p braced[2] = { { 3, 'c', 30L }, { 4 } };
int grid[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
int flat[2][2] = { 1, 2, 3, 4 };
int target = 9;
int *ptr = &target;
int *elem = &full[1];
char *text = "xyz";
long lng = 100000L;
long neg = -1;
double dbl = 2.5;
float fval = -0.75;
int zeroed[4];
long zl;
calc(v) { return v * 3; }
main()
{
	int a = 4;
	int b = a + 1;
	int c = calc(b);
	static int st = 11;
	if (s1 != 5 || s2 != -3) return 1;
	if (full[0] != 1 || full[2] != 3) return 2;
	if (part[1] != 8 || part[2] != 0 || part[4] != 0) return 3;
	if (sizeof sized != 8 || sized[3] != 7) return 4;
	if (sizeof str != 4 || str[2] != 'c' || str[3] != 0) return 5;
	if (sizeof fixed != 6 || fixed[1] != 'i' || fixed[2] != 0 || fixed[5] != 0) return 6;
	if (one.x != 1 || one.c != 'q' || one.l != 70000L) return 7;
	if (many[1].x != 2 || many[1].c != 'b' || many[1].l != 20L) return 8;
	if (braced[0].l != 30L || braced[1].x != 4 || braced[1].c != 0 || braced[1].l != 0) return 9;
	if (grid[1][0] != 4 || grid[1][2] != 6 || flat[1][0] != 3) return 10;
	if (*ptr != 9 || *elem != 2 || text[1] != 'y') return 11;
	if (lng != 100000L || neg != -1L || dbl != 2.5 || fval != -0.75) return 12;
	if (zeroed[0] != 0 || zeroed[3] != 0 || zl != 0) return 13;
	if (b != 5 || c != 15 || st != 11) return 14;
	return 0;
}
