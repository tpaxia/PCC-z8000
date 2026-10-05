typedef struct { int real; int imag; } complex_t;
int seen;
add(i,j) complex_t i,j; { complex_t tmp; tmp.real = i.real + j.real; tmp.imag = i.imag + j.imag; if (tmp.real != 15 || tmp.imag != 15) abort(); seen = 1; }
main() { complex_t i,j; i.real = 5; i.imag = 5; j.real = 10; j.imag = 10; add(i,j); if (seen != 1) abort(); return 0; }
