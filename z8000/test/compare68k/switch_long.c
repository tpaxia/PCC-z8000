match(x) long x;
{
 switch(x) {
 case 0x10001L: return 11;
 case 0x20001L: return 22;
 default: return 33;
 }
}
main()
{
 if(match(0x10001L)!=11) return 1;
 if(match(0x20001L)!=22) return 2;
 if(match(0x30001L)!=33) return 3;
 return 0;
}
