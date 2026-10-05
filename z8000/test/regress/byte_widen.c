unsigned char u;
char s;
main()
{
	unsigned long x;
	long y;
	int i;
	for(i=0;i<256;i++) {
		u=i; s=i;
		x=u; y=u;
		if(x!=i || y!=i) return 1;
		y=s;
		if(i<128) { if(y!=i) return 2; }
		else { if(y!=i-256) return 3; }
		x=s;
		if((long)x!=y) return 4;
	}
	return 0;
}
