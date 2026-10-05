char c;
short s;
long l;
main()
{
 c = -2; s = -300; l=100000L;
 if(!(c<l) || !(s<l) || l<=c || l<=s) return 1;
 if((c<l)!=1 || (s>l)!=0) return 2;
 return 0;
}
