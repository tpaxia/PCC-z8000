match(x) unsigned x;
{
 switch(x) {
 case 1: return 1;
 case 32767: return 2;
 case 32768L: return 3;
 case 40000L: return 4;
 case 50000L: return 5;
 case 65535L: return 6;
 default: return 7;
 }
}
main()
{
 if(match(1)!=1 || match(32767)!=2 || match((unsigned)32768L)!=3) return 1;
 if(match((unsigned)40000L)!=4 || match((unsigned)50000L)!=5 || match((unsigned)65535L)!=6) return 2;
 return 0;
}
