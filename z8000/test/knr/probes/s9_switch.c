/* KNR: 9.7.switch 9.7.default 9.7.fallthrough 9.7.no-match 9.7.char-and-negative 9.7.nested 9.7.long 15.case */
pick(v)
{
	switch (v) {
	case 1: return 10;
	case 2:
	case 3: return 23;
	default: return 99;
	case -4: return 40;
	case 'a': return 97;
	case 1000: return 1000;
	case 2 * 3 + 1: return 7;
	}
}
fall(v)
{
	int r;
	r = 0;
	switch (v) {
	case 0: r += 1;
	case 1: r += 2;
	case 2: r += 4; break;
	case 3: r += 8;
	}
	return r;
}
dense(v)
{
	switch (v) {
	case 0: return 5; case 1: return 6; case 2: return 7; case 3: return 8;
	case 4: return 9; case 5: return 10; case 6: return 11; case 7: return 12;
	}
	return -1;
}
main()
{
	int i, n;
	long l;
	if (pick(1) != 10 || pick(2) != 23 || pick(3) != 23 || pick(4) != 99) return 1;
	if (pick(-4) != 40 || pick('a') != 97 || pick(1000) != 1000 || pick(7) != 7) return 2;
	if (fall(0) != 7 || fall(1) != 6 || fall(2) != 4 || fall(3) != 8 || fall(9) != 0) return 3;
	for (i = 0; i < 8; i++) if (dense(i) != i + 5) return 4;
	if (dense(8) != -1 || dense(-1) != -1) return 5;
	n = 0;
	for (i = 0; i < 4; i++) {
		switch (i) {
		case 1: continue;
		case 2: switch (n) { case 1: n += 10; break; default: n += 100; } break;
		default: n++;
		}
		n += 1000;
	}
	if (n != 3102) return 6;
	switch (5) { }
	l = 70000L;
	switch (l) { case 4464: return 7; case 70000L: n = 1; break; default: return 8; }
	if (n != 1) return 9;
	return 0;
}
