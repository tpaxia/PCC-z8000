enum color { red = -1, green=1, blue=300 };
static enum color c=blue;
main()
{ enum color v; v=red; if(c!=300 || v!=-1) return 1; return 0; }
