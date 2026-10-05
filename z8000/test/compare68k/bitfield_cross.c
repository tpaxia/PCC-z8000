struct fields { unsigned a:8; unsigned :0; unsigned b:16; unsigned c:1; } x={255,65535L,1};
main()
{ if(x.a!=255 || x.b!=65535L || x.c!=1) return 1; return 0; }
