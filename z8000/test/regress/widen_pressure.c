/* Integer widening with four register variables and another live long. */
struct symbol { char name[8]; unsigned type; char a,b,c; int offset; short dim,size,use; };
struct stack { int size,x,n,s,d; unsigned t; int id,flag; long off; };
struct symbol symbols[2];
struct stack levels[2], *top;
long result;
record(a,b,c,d,e) int a,b,c,d; long e; {result=e;}
probe()
{
 register int t, ix, n, id;
 struct symbol *p;
 t=1; ix=2; n=3; id=t;
 p = &symbols[id];
 record(id,p->type,p->dim,p->size,p->offset+top->off);
}
struct usymbol { char name[8]; unsigned type; char a,b,c; unsigned offset; short dim,size,use; };
struct usymbol usyms[2];
uprobe()
{
 register int t, ix, n, id;
 struct usymbol *p;
 t=1; ix=2; n=3; id=t;
 p = &usyms[id];
 record(id,p->type,p->dim,p->size,p->offset+top->off);
}
main()
{
 top=levels;
 symbols[1].offset = -123;
 top->off=100000L;
 probe();
 if(result != 99877L) return 1;
 usyms[1].offset=65000;
 uprobe();
 if(result != 165000L) return 2;
 return 0;
}
