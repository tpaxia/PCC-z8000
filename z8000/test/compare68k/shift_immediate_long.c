unsigned long u;
main()
{
 u=1L; if((u<<17)!=0x20000L) return 1;
 if((u<<31)!=0x80000000L) return 2;
 u=0x80000000L; if((u>>31)!=1L) return 3;
 return 0;
}
