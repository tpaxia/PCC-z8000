long id(x) long x; { return x; }
long add(a,b,c) long a,b,c; { return a+2L*b+3L*c; }
main()
{ if(add(id(0x10001L),id(0x20002L),id(0x30003L))!=0xe000eL) return 1; return 0; }
