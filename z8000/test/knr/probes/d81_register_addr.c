/* KNR: 8.1.register-no-address
 * EXPECT-ERROR:
 * "the address-of operator & cannot be applied to them" */
main()
{
	register int r;
	int *p;
	r = 1;
	p = &r;
	return *p;
}
