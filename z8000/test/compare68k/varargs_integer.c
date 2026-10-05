/* Expanded equivalent of varargs.h, with default-promoted types. */
check(first)
int first;
{
 char *ap;
 long l;
 int i;
 unsigned u;
 ap=(char *)&first;
 if(*(int *)ap!=3) return 1;
 ap+=sizeof(int); l = *(long *)ap;
 ap+=sizeof(long); i = *(int *)ap;
 ap+=sizeof(int); u = *(unsigned *)ap;
 if(l!=0x12345678L || i!=-7 || u!=60000L) return 2;
 return 0;
}
main()
{ return check(3,0x12345678L,-7,(unsigned)60000L); }
