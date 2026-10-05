char s;
unsigned char u;
char *p;
main()
{
 s = -2; u=250;
 p=(char *)s; if((unsigned)p!=65534L) return 1;
 p=(char *)u; if((unsigned)p!=250) return 2;
 return 0;
}
