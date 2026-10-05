int a[8];
struct pair { int a; long b; } pairs[4];
main()
{
	int *p, *q;
	struct pair *s;
	p = &a[2]; q = &a[7];
	if (q - p != 5 || p - q != -5) return 1;
	s = &pairs[3];
	if (s - pairs != 3) return 2;
	if (sizeof(struct pair) != 6) return 3;
	return 0;
}
