/* KNR: 8.7.type-names */
int arr[3];
seven() { return 7; }
main()
{
	char *cp;
	int i;
	if (sizeof(int *) != 2 || sizeof(char **) != 2) return 1;
	if (sizeof(int *[3]) != 6) return 2;		/* array of 3 pointers */
	if (sizeof(int (*)[3]) != 2) return 3;		/* pointer to array of 3 */
	if (sizeof(int (*)()) != 2) return 4;		/* pointer to function */
	if (sizeof(long [5]) != 20 || sizeof(char [2][3]) != 6) return 5;
	cp = (char *)seven;
	if ((*(int (*)())cp)() != 7) return 6;
	arr[2] = 5; cp = (char *)arr;
	if ((*(int (*)[3])cp)[2] != 5) return 7;
	i = 0x141; i = (int)(char)i;
	if (i != 0x41) return 8;
	return 0;
}
