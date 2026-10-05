/* Defined 32-bit unsigned wrap on an LP64 host. */
main()
{
 if(((unsigned long)0xffffffffL+1L)!=0L) return 1;
 if(((unsigned long)0x80000000L<<1)!=0L) return 2;
 if((~(unsigned long)0L)!=(unsigned long)0xffffffffL) return 3;
 return 0;
}
