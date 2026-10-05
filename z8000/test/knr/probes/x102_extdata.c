/* KNR: 10.2.external-data 10.2.static-file 10.2.extern-then-define 11.2.forward-reference */
extern int later;
extern int table[];
static int priv = 3;
static helper();
int count;
use() { return later + table[1] + helper(); }
int later = 10;
int table[3] = { 1, 2, 3 };
static helper() { return priv + count; }
main()
{
	extern int count;
	count = 4;
	if (use() != 19) return 1;
	if (sizeof table != 6) return 2;
	return 0;
}
