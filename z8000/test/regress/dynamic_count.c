int sink;
main()
{
	register int n;
	int x;
	n = 4; x = 256;
	sink = x >> n;
	if (n != 4) return 1;
	return 0;
}
