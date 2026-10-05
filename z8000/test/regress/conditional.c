int calls;
touch(x) int x; { ++calls; return x; }
main()
{
	int x, y;
	long l;
	x = 1;
	y = x ? touch(7) : touch(9);
	if (y != 7 || calls != 1) return 1;
	x = 0;
	y = x ? touch(7) : touch(9);
	if (y != 9 || calls != 2) return 2;
	l = x ? 123456L : 654321L;
	if (l != 654321L) return 3;
	y = (touch(3), touch(4));
	if (y != 4 || calls != 4) return 4;
	return 0;
}
