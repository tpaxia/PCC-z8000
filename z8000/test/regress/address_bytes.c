/* Relocate the entire address before narrowing it to a byte. */
char padding[300];
char target;
main()
{
	char a;
	unsigned char b;
	char *p;
	unsigned word;
	p = &target;
	word = (unsigned)p;
	a = (char)&target;
	b = (unsigned char)&target;
	if (a != (char)word || b != (unsigned char)word) return 1;
	a = (char)(&target + 3);
	b = (unsigned char)(&target + 3);
	if (a != (char)(word + 3) || b != (unsigned char)(word + 3)) return 2;
	a = &target;
	if (a != (char)word) return 3;
	return 0;
}
