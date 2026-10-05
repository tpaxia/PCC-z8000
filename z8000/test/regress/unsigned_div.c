main()
{
	unsigned a, b, q, r;
	a = 65535L; b = 3;
	q = a / b; r = a % b;
	if (q != 21845 || r != 0) return 1;
	b = 40000L;
	q = a / b; r = a % b;
	if (q != 1 || r != 25535) return 2;
	return 0;
}
