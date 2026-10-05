/* KNR: 8.5.struct 8.5.nested 8.5.self-reference 8.5.layout 8.5.array-member 8.5.tag-reuse */
struct inner { char tag; int val; };
struct outer { int id; struct inner in; char name[5]; long big; };
struct node { int v; struct node *next; };
struct outer arr[3];
main()
{
	struct outer o, *op;
	struct node a, b, c, *np;
	struct inner i2;
	int sum;
	if (sizeof(struct inner) != 4) return 1;	/* char, pad, int */
	if (sizeof(struct outer) != 16) return 2;	/* 2 + 4 + 5 + pad + 4 */
	o.id = 1; o.in.tag = 't'; o.in.val = 300; o.name[4] = 'z'; o.big = 70000L;
	op = &o;
	if (op->id != 1 || op->in.tag != 't' || op->in.val != 300 || op->name[4] != 'z' || op->big != 70000L) return 3;
	a.v = 1; b.v = 2; c.v = 3; a.next = &b; b.next = &c; c.next = 0;
	for (sum = 0, np = &a; np; np = np->next) sum += np->v;
	if (sum != 6 || a.next->next->v != 3) return 4;
	arr[2].in.val = 9; op = arr;
	if ((op + 2)->in.val != 9 || op[2].in.val != 9) return 5;
	if ((char *)&arr[1] - (char *)&arr[0] != 16) return 6;
	i2.val = 4;
	if ((char *)&i2.val - (char *)&i2 != 2) return 7;
	return 0;
}
