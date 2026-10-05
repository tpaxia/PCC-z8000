unsigned long raw;
char *p;
main()
{
 raw=0x12345678L; p=(char *)raw;
 if((unsigned)p!=0x5678) return 1;
 raw=(unsigned long)p;
 if(raw!=0x5678L) return 2;
 return 0;
}
