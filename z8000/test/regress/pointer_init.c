int a[3] = { 7, 8, 9 };
int *p = &a[1];
main()
{
	if (*p != 8) return 1;
	return 0;
}
