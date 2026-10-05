long id(x) long x; { return x; }
main()
{
 register long a,b;
 a=0x10001L; b=0x20002L;
 if(a+id(b)+id(a)!=0x40004L) return 1;
 if(a!=0x10001L || b!=0x20002L) return 2;
 return 0;
}
