int add(i,j) int i,j; { return i*j; }
main() { if (add(add(1,2),add(3,add(4,5))) != 120) return 1; return 0; }
