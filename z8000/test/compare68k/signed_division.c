int a,b;
main()
{
 a = -30000; b=7;
 if(a/b!=-4285 || a%b!=-5) return 1;
 b = -7; if(a/b!=4285 || a%b!=-5) return 2;
 a=30000; if(a/b!=-4285 || a%b!=5) return 3;
 a/=b; if(a!=-4285) return 4;
 a = -30000; a%=b; if(a!=-5) return 5;
 return 0;
}
