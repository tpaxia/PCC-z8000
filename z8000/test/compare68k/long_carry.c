unsigned long a,b;
main()
{
 a=0xffffL; b=1L;
 if(a+b!=0x10000L) return 1;
 a=0x10000L; if(a-b!=0xffffL) return 2;
 a=0xffffffffL; a+=b; if(a!=0L) return 3;
 a-=b; if(a!=0xffffffffL) return 4;
 return 0;
}
