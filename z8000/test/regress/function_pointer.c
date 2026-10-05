add(a, b) int a, b; { return a + b; }
main()
{
	int (*f)();
	f = add;
	if ((*f)(7, 3) != 10) return 1;
	return 0;
}
