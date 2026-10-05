/* A long initialized from an address holds that address in its low word. */
int y;
int arr[4];
long x1 = (long)&y;
long x2 = (long)&arr[2];
long x3 = (long)arr;
long plain = 70000L;
main()
{
	if (x1 != (long)(unsigned)&y) return 1;
	if (x2 != (long)(unsigned)&arr[2]) return 2;
	if (x3 != (long)(unsigned)arr) return 3;
	if ((int *)(int)x2 != &arr[2]) return 4;
	if (plain != 70000L) return 5;
	return 0;
}
