id(x) int x; { return x; }
add(a,b,c,d) int a,b,c,d; { return a+2*b+3*c+4*d; }
main()
{ if(add(id(1),id(2),id(3),id(4))!=30) return 1; return 0; }
