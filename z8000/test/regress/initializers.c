struct pair { int a; long b; };
struct pair pairs[2] = { { 7, 65538L }, { -3, -65539L } };
int a[5] = { 1, 2 };
char s[] = "a\n\101";
int *p = &a[1];
main()
{
	if (pairs[0].a != 7 || pairs[0].b != 65538L) return 1;
	if (pairs[1].a != -3 || pairs[1].b != -65539L) return 2;
	if (a[2] || a[3] || a[4]) return 3;
	if (s[0] != 'a' || s[1] != 10 || s[2] != 'A' || s[3]) return 4;
	if (*p != 2 || sizeof(s) != 4) return 5;
	return 0;
}
