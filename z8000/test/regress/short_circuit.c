int calls;
touch() { ++calls; return 1; }
main()
{
	int x, y;
	x = 0;
	y = 1;
	if (x && touch()) return 1;
	if (!(y || touch())) return 2;
	if (calls != 0) return 3;
	if (!(y && touch())) return 4;
	if (!(x || touch())) return 5;
	if (calls != 2) return 6;
	return 0;
}
