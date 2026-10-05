/* KNR: 8.5.field-width 8.5.field-unnamed 8.5.field-zero 8.5.field-truncation */
struct f { unsigned a : 3; unsigned b : 5; unsigned : 2; unsigned c : 6; };
struct z { unsigned a : 4; unsigned : 0; unsigned b : 4; };
main()
{
	struct f x;
	struct z y;
	int i;
	if (sizeof(struct f) != 2) return 1;
	if (sizeof(struct z) != 4) return 2;	/* width 0 starts a new word */
	x.a = 5; x.b = 21; x.c = 42;
	if (x.a != 5 || x.b != 21 || x.c != 42) return 3;
	x.a = 15;
	if (x.a != 7 || x.b != 21) return 4;	/* truncated to 3 bits, neighbour intact */
	x.b = 0;
	if (x.a != 7 || x.c != 42) return 5;
	x.c++;
	x.a += 2;
	if (x.c != 43 || x.a != 1) return 6;
	i = x.c * 2 + x.a;
	if (i != 87) return 7;
	y.a = 9; y.b = 6;
	if (y.a != 9 || y.b != 6) return 8;
	return 0;
}
