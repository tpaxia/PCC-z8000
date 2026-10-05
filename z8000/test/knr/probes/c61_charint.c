/* KNR: 6.1.char-to-int 6.1.int-to-char 6.1.sign-extension */
main()
{
	char c;
	int i;
	c = 'A'; i = c;
	if (i != 65) return 1;
	i = 300; c = i;
	if (c != 44) return 2;
	i = 0x1234; c = i;
	if (c != 0x34) return 3;
	/* machine dependent: this implementation sign-extends char */
	c = '\377'; i = c;
	if (i != -1) return 4;
	c = 200;
	if (c >= 0) return 5;
	return 0;
}
