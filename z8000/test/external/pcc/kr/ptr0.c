char p = 'A';
main() { char *t1[3]; int off; char *z; t1[0] = &p; t1[1] = &p; t1[2] = &p; off = 2; z = t1[off]; if (*z != 'A' || z != &p) abort(); return 0; }
