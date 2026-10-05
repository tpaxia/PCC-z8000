/* KNR: 14.1.member-address V7.struct-assign V7.struct-argument V7.struct-return */
struct pt { int x; long y; char z; };
struct big { int a[20]; };
struct pt make(x, y) long y; { struct pt p; p.x = x; p.y = y; p.z = 'm'; return p; }
total(p) struct pt p; { p.x += 100; return p.x + (int)p.y + p.z; }
struct big fill(v) { struct big b; int i; for (i = 0; i < 20; i++) b.a[i] = v + i; return b; }
struct pt garr[2];
main()
{
	struct pt a, b, *p;
	struct big g, h;
	int *ip;
	a.x = 1; a.y = 70000L; a.z = 'a';
	b = a;
	if (b.x != 1 || b.y != 70000L || b.z != 'a') return 1;
	b.x = 2;
	if (a.x != 1) return 2;
	p = &b; *p = a; garr[1] = *p;
	if (garr[1].y != 70000L || p->x != 1) return 3;
	a = make(5, 6L);
	if (a.x != 5 || a.y != 6 || a.z != 'm') return 4;
	if (total(a) != 105 + 6 + 'm' || a.x != 5) return 5;
	g = fill(3); h = g;
	if (h.a[0] != 3 || h.a[19] != 22) return 7;
	ip = &a.x;
	if (*ip != 5 || (char *)&a.z - (char *)&a != 6) return 8;
	return 0;
}
