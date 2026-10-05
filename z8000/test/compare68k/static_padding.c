struct padded { char a; long b; char c; } x={7,0x12345678L,9};
char *p;
main()
{
 if(x.a!=7 || x.b!=0x12345678L || x.c!=9) return 1;
 p=(char *)&x;
 if((char *)&x.b-p!=2 || (char *)&x.c-p!=6) return 2;
 return 0;
}
