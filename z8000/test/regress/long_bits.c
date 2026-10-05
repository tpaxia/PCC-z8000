main()
{
	long a, b, c;
	a = 305419896L; b = 16711935L;
	c = a & b;
	if (c != 3407992L) return 1;
	c = a | b;
	if (c != 318723839L) return 2;
	c = a ^ b;
	if (c != 315315847L) return 3;
	return 0;
}
