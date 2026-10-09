/* Offsets are in bits: a 5 KiB result crosses a native 16-bit int. */
struct big { int words[2560]; };
struct big make(n)
int n;
{
 struct big r; int i;
 for(i=0;i<2560;i++)r.words[i]=n+i;
 return r;
}
check(p, n)
int *p, n;
{
 int i;
 for(i=0;i<2560;i++)if(p[i]!=n+i)return 1;
 return 0;
}
main()
{
 struct big (*fp)();
 fp=make;
 if(check((*fp)(17).words,17))return 1;
 if(check(make(100).words,100))return 2;
 return 0;
}
