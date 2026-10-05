long add(x,y) long x,y; { return x+y; }
long (*fp)();
main()
{ fp=add; if((*fp)(0x10001L,0x20002L)!=0x30003L) return 1; return 0; }
