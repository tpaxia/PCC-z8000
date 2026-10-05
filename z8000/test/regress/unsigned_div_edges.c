/* Cross sign/quotient boundaries, register operands and compound updates. */
unsigned nums[]={0,1,2,32767,32768L,40000L,65534L,65535L};
unsigned dens[]={1,2,3,32767,32768L,40000L,65535L};
main()
{
	register unsigned a,b;
	unsigned q,r,x;
	unsigned long want;
	int i,j;
	for(i=0;i<8;i++) for(j=0;j<7;j++) {
		a=nums[i]; b=dens[j];
		want=(unsigned long)a/(unsigned long)b;
		q=a/b; r=a%b;
		if(q!=want || (unsigned long)q*b+r!=a || r>=b) return 1;
		if(b!=dens[j] || a!=nums[i]) return 2;
		x=a; x/=b; if(x!=q) return 3;
		x=a; x%=b; if(x!=r) return 4;
	}
	return 0;
}
