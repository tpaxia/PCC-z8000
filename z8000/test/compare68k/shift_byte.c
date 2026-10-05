char c;
unsigned char u;
int n;
main()
{
 c = -64; u=128; n=2;
 c>>=n; u>>=n;
 if(c!=-16 || u!=32 || n!=2) return 1;
 u<<=n; if(u!=128) return 2;
 return 0;
}
