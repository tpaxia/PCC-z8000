/* K&R default promotions, updates, signed zero and unordered comparisons. */
union dbits { double d; unsigned short w[4]; } a,b;
union fbits { float f; unsigned short w[2]; } x;
double keep(v) double v; { return v; }
main()
{
    double d,old;
    float f;
    long l;
    unsigned long u;
    int i;
    unsigned n;
    f=3.0; if(keep(f)!=3.0) return 1;
    d=6.0; d*=2.0; d/=3.0; d-=1.0; d+=2.0;
    if(d!=5.0) return 2;
    old=d++; if(old!=5.0 || d!=6.0) return 3;
    old=d--; if(old!=6.0 || d!=5.0) return 4;
    f*=2.0; f/=3.0; f+=1.0; f-=2.0;
    if(f!=1.0) return 5;
    old=f++; if(old!=1.0 || f!=2.0) return 6;
    old=f--; if(old!=2.0 || f!=1.0) return 7;
    l = -2147483647L; d=l; l=d; if(l!=-2147483647L) return 8;
    u=0xffffffffL; d=u; u=d; if(u!=0xffffffffL) return 9;
    i = -32768; d=i; i=d; if(i!=-32768) return 10;
    n=(unsigned)65535L; d=n; n=d; if(n!=(unsigned)65535L) return 11;
    a.w[0]=0x8000; a.w[1]=a.w[2]=a.w[3]=0;
    if(a.d || a.d!=0.0 || !(a.d==0.0)) return 12;
    a.w[0]=0x7ff8; a.w[3]=1;
    if(!a.d || a.d==a.d || !(a.d!=a.d)) return 13;
    if(a.d<0.0 || a.d<=0.0 || a.d>0.0 || a.d>=0.0) return 14;
    x.w[0]=0x8000; x.w[1]=0; if(x.f) return 15;
    x.w[0]=0x7fc0; if(!x.f || x.f==x.f) return 16;
    f=1.0; d=0.000000059604644775390625;
    d+=0.0000000000000008881784197001252;
    f+=d; x.f=f;
    if(x.w[0]!=0x3f80 || x.w[1]!=1) return 17;
    old = ++f; if(old!=2.0 || f!=old) return 18;
    old = --f; if(old!=1.0 || f!=old) return 19;
    return 0;
}
