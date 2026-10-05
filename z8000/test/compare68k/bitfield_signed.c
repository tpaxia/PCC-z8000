struct fields { int a:3,b:5,c:8; } x;
main()
{
 x.a = -2; x.b = -3; x.c = -128;
 if(x.a!=-2 || x.b!=-3 || x.c!=-128) return 1;
 x.b++; if(x.b!=-2) return 2;
 return 0;
}
