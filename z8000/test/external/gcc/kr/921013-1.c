f(d,x,y,n)
int*d;
float*x,*y;
int n;
{
  while(n--){*d++= *x++==*y++;}
}

main()
{
  int r[4];
  float a[4];
  float b[4];
  int i;
  a[0]=5; a[1]=1; a[2]=3; a[3]=5;
  b[0]=2; b[1]=4; b[2]=3; b[3]=0;
  f(r,a,b,4);
  for(i=0;i<4;i++)
    if((a[i]==b[i])!=r[i])
      abort();
  exit(0);
}
