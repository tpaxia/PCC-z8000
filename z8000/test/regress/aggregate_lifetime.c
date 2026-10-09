/* Caller-owned results must survive another invocation of the same callee. */
struct pair { int x, y; };
struct large { int v[24]; };
union result { long value; int words[2]; };
struct pair make(n)
register int n;
{ struct pair p; p.x=n; p.y=n+1; return p; }
struct pair recurse(n)
int n;
{ struct pair p; if (!n) return make(3); p=recurse(n-1); p.x+=n; return p; }
combine(a,b)
struct pair a,b;
{ return a.x*100+a.y*10+b.x+b.y; }
struct large big(n)
int n;
{ struct large p; int i; for(i=0;i<24;i++)p.v[i]=n+i; return p; }
union result un(n)
long n;
{ union result u; u.value=n; return u; }
main()
{
 struct pair p; struct large b; union result u;
 struct pair (*fp)(); struct large (*bp)();
 fp=make; bp=big;
 if(combine(make(1),make(7))!=135)return 1;
 if(combine((*fp)(2),(*fp)(4))!=239)return 2;
 p=recurse(7); if(p.x!=31 || p.y!=4)return 3;
 b=(*bp)(12); if(b.v[0]!=12 || b.v[23]!=35)return 4;
 u=un(0x12345678L); if(u.value!=0x12345678L)return 5;
 if(make(make(8).y).x!=9)return 6;
 return 0;
}
