struct fields { unsigned a:4,b:4,c:8; } x;
main()
{
 x.a=7; x.b=9; x.c=123;
 x.b=0;
 if(x.a!=7 || x.b!=0 || x.c!=123) return 1;
 return 0;
}
