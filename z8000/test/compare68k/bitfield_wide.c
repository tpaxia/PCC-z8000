struct fields { unsigned a:1,b:15,c:8,d:8; } x;
main()
{
 x.a=1; x.b=32767; x.c=255; x.d=128;
 if(x.a!=1 || x.b!=32767 || x.c!=255 || x.d!=128) return 1;
 x.b-=2; x.c^=127; x.d++;
 if(x.b!=32765 || x.c!=128 || x.d!=129) return 2;
 return 0;
}
