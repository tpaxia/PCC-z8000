id(x) int x; { return x; }
main()
{
 register int a,b,c,d;
 register int *p,*q,*r;
 int v[3];
 a=11;b=22;c=33;d=44;p = &v[0];q = &v[1];r = &v[2];
 *p=101;*q=202;*r=303;
 if(a+id(b)+id(c)+id(d)!=110) return 1;
 if(*p+id(*q)+id(*r)!=606) return 2;
 return 0;
}
