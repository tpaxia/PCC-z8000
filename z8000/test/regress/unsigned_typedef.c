typedef short UNIT;
typedef char BYTE;
typedef long WIDE;
main()
{
	unsigned UNIT count;
	unsigned BYTE byte;
	unsigned WIDE wide;
	unsigned UNIT *p;
	count = 65535; byte = 255; wide = 0xffffffffL;
	p = &count;
	if (*p != 65535 || byte != 255 || wide / 2 != 2147483647L) return 1;
	{ unsigned UNIT; UNIT = 7; if (UNIT != 7) return 2; }
	return 0;
}
