/* A pair must fit below, rather than straddle, the last free register. */
probe()
{
 register int i;
 register long j, k;
 register int n;
 i=3; j=100000L; k=70000L; n=9;
 if(i!=3 || j!=100000L || k!=70000L || n!=9) return 1;
 for(i=0;i<5;i++) { j+=2; k-=3; }
 if(i!=5 || j!=100010L || k!=69985L || n!=9) return 2;
 return 0;
}
main() { return probe(); }
