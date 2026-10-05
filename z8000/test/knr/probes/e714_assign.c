/* KNR: 7.14.simple 7.14.chain 7.14.conversion 7.14.compound 7.14.lhs-once 7.14.pointer 7.14.long 7.14.float */
int a[4];
main()
{
	int i, j, k, *p;
	char c;
	long l;
	double d;
	if ((i = 5) != 5) return 1;
	i = j = k = 7;
	if (i != 7 || j != 7 || k != 7) return 2;
	if ((c = 300) != 44) return 3;		/* value is that of the left operand */
	i = 10;
	i += 5; if (i != 15) return 4;
	i -= 3; if (i != 12) return 5;
	i *= 4; if (i != 48) return 6;
	i /= 5; if (i != 9) return 7;
	i %= 5; if (i != 4) return 8;
	i <<= 3; if (i != 32) return 9;
	i >>= 2; if (i != 8) return 10;
	i |= 3; if (i != 11) return 11;
	i &= 6; if (i != 2) return 12;
	i ^= 7; if (i != 5) return 13;
	a[0] = 1; a[1] = 10; i = 0;
	a[i++] += 5;
	if (i != 1 || a[0] != 6 || a[1] != 10) return 14;
	p = a; p += 2;
	if (p != &a[2]) return 15;
	p -= 1;
	if (p != &a[1]) return 16;
	l = 65535L; l += 1; l *= 2; l -= 2; l /= 3;
	if (l != 43690L) return 17;
	l <<= 4; l |= 1; l %= 1000;
	if (l != 41) return 18;
	d = 1.5; d += 1; d *= 2; d -= 0.5; d /= 3;
	if (d != 1.5) return 19;
	i = 3; i += i += 1;	/* order unspecified, both common results accepted */
	if (i != 8 && i != 7) return 20;
	return 0;
}
