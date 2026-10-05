/* KNR: 14.4.pointer-integer-round-trip 14.4.pointer-type-cast 14.4.byte-access */
int words[2];
main()
{
	int i, *ip;
	char *cp;
	long l, *lp;
	unsigned u;
	ip = &words[1];
	u = (unsigned)ip; 
	if ((int *)u != ip) return 1;
	i = (int)ip;
	if ((int *)i != ip) return 2;
	cp = (char *)words;
	if ((int *)(cp + 2) != ip) return 3;
	l = 0x01020304L; cp = (char *)&l;
	/* machine dependent: high byte first */
	if (cp[0] != 1 || cp[1] != 2 || cp[2] != 3 || cp[3] != 4) return 4;
	lp = (long *)cp;
	if (*lp != 0x01020304L) return 5;
	words[0] = 0x1122; words[1] = 0x3344;
	if (*(long *)words != 0x11223344L) return 6;
	ip = (int *)&l;
	if (ip[0] != 0x0102 || ip[1] != 0x0304) return 7;
	return 0;
}
