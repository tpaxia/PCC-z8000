/* KNR: 7.2.preinc 7.2.postinc 7.2.incdec-pointer 7.2.incdec-char-long */
struct s { int m; } st;
long la[3];
main()
{
	int i, a[3];
	char c;
	long l, *lp;
	i = 5;
	if (++i != 6 || i != 6) return 1;
	if (i++ != 6 || i != 7) return 2;
	if (--i != 6 || i-- != 6 || i != 5) return 3;
	c = 'a';
	if (++c != 'b' || c++ != 'b' || c != 'c') return 4;
	l = 65535L;
	if (++l != 65536L) return 5;
	if (l-- != 65536L || l != 65535L) return 6;
	lp = la;
	if (++lp != &la[1] || lp++ != &la[1] || lp != &la[2] || --lp != &la[1]) return 7;
	st.m = 1;
	if (st.m++ != 1 || ++st.m != 3) return 8;
	a[0] = 0; a[1] = 10; a[2] = 20; i = 0;
	if (a[i++] != 0 || a[i]++ != 10 || a[1] != 11 || i != 1) return 9;
	return 0;
}
