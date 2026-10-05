long a[3];
main()
{
 long *p;
 p=a; a[0]=0x10001L; a[1]=0x20002L;
 *p++ += 0x10002L;
 if(p!=a+1 || a[0]!=0x20003L) return 1;
 *p++ *= 3L;
 if(p!=a+2 || a[1]!=0x60006L) return 2;
 *--p /= 2L;
 if(p!=a+1 || a[1]!=0x30003L) return 3;
 return 0;
}
