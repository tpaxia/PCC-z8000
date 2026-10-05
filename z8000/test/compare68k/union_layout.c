union word { long l; unsigned short w[2]; unsigned char c[4]; } x;
main()
{
 x.l=0x12345678L;
 if(x.w[0]!=0x1234 || x.w[1]!=0x5678) return 1;
 if(x.c[0]!=0x12 || x.c[1]!=0x34 || x.c[2]!=0x56 || x.c[3]!=0x78) return 2;
 return 0;
}
