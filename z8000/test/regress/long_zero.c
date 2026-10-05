/* Both words must participate in a long truth test. */
main()
{
	long x;
	x = 65536L;
	if (x == 0L) return 1;
	if (!x) return 2;
	x = 0L;
	if (x != 0L) return 3;
	return 0;
}
