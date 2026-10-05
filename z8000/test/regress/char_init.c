char a[2] = { 1, 42 };
char s[] = "ABC";
main()
{
	if (a[0] != 1 || a[1] != 42) return 1;
	if (s[0] != 'A' || s[1] != 'B' || s[2] != 'C' || s[3]) return 2;
	return 0;
}
