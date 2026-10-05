/* Byte increment must not change the adjacent byte. */
char a[2];
main()
{
	int old;
	a[0] = 1; a[1] = 42;
	old = a[0]++;
	if (old != 1) return 1;
	if (a[0] != 2) return 2;
	if (a[1] != 42) return 3;
	old = a[0]--;
	if (old != 2 || a[0] != 1 || a[1] != 42) return 4;
	return 0;
}
