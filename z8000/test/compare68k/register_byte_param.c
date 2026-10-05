keep(x,y)
register char x;
register unsigned char y;
{ if(x != -2) return 1; if(y != 250) return 2; return 0; }
main()
{ return keep(-2,250); }
