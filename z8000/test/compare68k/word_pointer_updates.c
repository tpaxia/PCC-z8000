int a[3];
main()
{
 int *p;
 p=a; a[0]=101; a[1]=201;
 *p++ += 2;
 if(p!=a+1 || a[0]!=103) return 1;
 *p++ *= 3;
 if(p!=a+2 || a[1]!=603) return 2;
 *--p /= 3;
 if(p!=a+1 || a[1]!=201) return 3;
 return 0;
}
