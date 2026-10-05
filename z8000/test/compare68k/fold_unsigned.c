/* Defined 16-bit unsigned wrap must also hold during constant folding. */
main()
{
 if(((unsigned)65535L+1)!=0) return 1;
 if(((unsigned)32768L<<1)!=0) return 2;
 if((~(unsigned)0)!=(unsigned)65535L) return 3;
 return 0;
}
