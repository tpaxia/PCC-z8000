/* KNR: 2.1.between-tokens 2.1.contents */
int a/**/= /* one */4/* two
   lines */;
main()
{
	int b;
	b = a /**/ / /**/ 2;	/* a comment may hold " and ' and // freely */
	if (b != 2) return 1;
	b = a/* */+/* */a;
	if (b != 8) return 2;
	return 0;
}
