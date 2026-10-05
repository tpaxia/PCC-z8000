/* KNR: 8.4.pointer 8.4.array 8.4.function 8.4.array-of-pointers 8.4.pointer-to-array 8.4.pointer-to-function 8.4.function-returning-pointer 8.4.multidimensional */
int x = 1, y = 2, z = 3;
int *ap[3];
int (*pa)[3];
int grid[2][3];
int (*pf)();
int *fp();
int (*apf[2])();
char **pp;
char *names[2];
one() { return 1; }
two() { return 2; }
int *fp() { return &y; }
main()
{
	ap[0] = &x; ap[1] = &y; ap[2] = &z;
	if (*ap[0] + *ap[1] + *ap[2] != 6 || sizeof ap != 6) return 1;
	grid[1][0] = 10; grid[1][2] = 12;
	pa = grid;
	if (sizeof pa != 2 || (*(pa + 1))[2] != 12 || pa[1][0] != 10) return 2;
	if (sizeof grid != 12 || sizeof grid[0] != 6) return 3;
	pf = one;
	if ((*pf)() != 1) return 4;
	pf = two;
	if ((*pf)() != 2) return 5;
	if (*fp() != 2) return 6;
	apf[0] = one; apf[1] = two;
	if ((*apf[0])() + (*apf[1])() != 3) return 7;
	names[0] = "ab"; names[1] = "cd"; pp = names;
	if (**pp != 'a' || *pp[1] != 'c' || pp[1][1] != 'd' || *(*(pp + 1) + 1) != 'd') return 8;
	return 0;
}
