long sum(a, b, c) int a; long b; char c;
{ return a + b + c; }
main()
{
	long result;
	result = sum(-7, 65536L, -2);
	if (result != 65527L) return 1;
	return 0;
}
