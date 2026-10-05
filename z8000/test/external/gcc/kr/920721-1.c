long f(a,b) short a,b;{return (long)a/b;}
main(){if(f((short)-32768,(short)-1)!=32768L)abort();else exit(0);}
