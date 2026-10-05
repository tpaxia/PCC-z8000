/* KNR: 2.2.case 2.2.underscore 2.2.long-names */
int abc = 1, ABC = 2, Abc = 3;
int _x = 4, a_b1 = 5, __ = 6;
int quitelongidentifier = 7;
main()
{
	if (abc != 1 || ABC != 2 || Abc != 3) return 1;
	if (_x != 4 || a_b1 != 5 || __ != 6) return 2;
	if (quitelongidentifier != 7) return 3;
	return 0;
}
