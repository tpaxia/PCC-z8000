long a,b;
main()
{
 a = -1000000L; b=7L;
 if(a/b!=-142857L || a%b!=-1L) return 1;
 b = -7L; if(a/b!=142857L || a%b!=-1L) return 2;
 a=1000000L; if(a/b!=-142857L || a%b!=1L) return 3;
 a/=b; if(a!=-142857L) return 4;
 a = -1000000L; a%=b; if(a!=-1L) return 5;
 return 0;
}
