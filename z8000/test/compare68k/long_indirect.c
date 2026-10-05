long a[3];
main()
{
 register long *p;
 p=a; p[0]=0x12345678L; p[1]=0x23456789L; p[2]=0x3456789aL;
 if(*p!=0x12345678L) return 1;
 if(*(p+1)!=0x23456789L) return 2;
 *(p+2) = *(p+1);
 if(p[2]!=0x23456789L) return 3;
 return 0;
}
