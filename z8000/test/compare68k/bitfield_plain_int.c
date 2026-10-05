/* Plain int bit-fields are unsigned, as in the original compiler: a field is
 * extracted by shift and mask with no sign extension. */
struct fields { int a:3,b:5,c:8; } x;
struct flags { int one:1; int two:2; } f;
main()
{
 x.a = -2; x.b = -3; x.c = -128;
 if(x.a!=6 || x.b!=29 || x.c!=128) return 1;
 x.b++; if(x.b!=30) return 2;
 f.one = 1; f.two = 2;
 if(f.one!=1 || f.two!=2) return 3;
 if(x.a<0 || f.one<0) return 4;
 return 0;
}
