struct chunk { int a,b,c; };
check(x,after) struct chunk x; int after;
{ if(x.a!=11 || x.b!=22 || x.c!=33 || after!=44) return 1; return 0; }
main()
{ struct chunk x; x.a=11; x.b=22; x.c=33; return check(x,44); }
