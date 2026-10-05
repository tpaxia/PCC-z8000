struct chunk { int a,b,c; };
struct guard { struct chunk data; int tail[3]; } src,dst;
main()
{
 src.data.a=11; src.data.b=22; src.data.c=33;
 src.tail[0]=1; src.tail[1]=2; src.tail[2]=3;
 dst.tail[0]=91; dst.tail[1]=92; dst.tail[2]=93;
 dst.data=src.data;
 if(dst.data.a!=11 || dst.data.b!=22 || dst.data.c!=33) return 1;
 if(dst.tail[0]!=91 || dst.tail[1]!=92 || dst.tail[2]!=93) return 2;
 return 0;
}
