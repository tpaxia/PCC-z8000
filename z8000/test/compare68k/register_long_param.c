long keep(x)
register long x;
{ return x; }
main()
{ if(keep(0x12345678L)!=0x12345678L) return 1; return 0; }
