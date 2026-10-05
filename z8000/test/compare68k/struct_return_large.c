struct chunk { int a,b,c; };
struct guard { struct chunk data; int tail[3]; } dst;
struct chunk make(x)
int x;
{
 struct chunk v;
 v.a=x; v.b=x+1; v.c=x+2;
 return v;
}
main()
{
 dst.tail[0]=91; dst.tail[1]=92; dst.tail[2]=93;
 dst.data=make(11);
 if(dst.data.a!=11 || dst.data.b!=12 || dst.data.c!=13) return 1;
 if(dst.tail[0]!=91 || dst.tail[1]!=92 || dst.tail[2]!=93) return 2;
 return 0;
}
