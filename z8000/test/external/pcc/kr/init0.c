char *str = "string";
char **strp = &str;

struct template {
	int i;
	int j;
} s;

struct template *r = &s;

main() { if (strp != &str || (*strp)[0] != 's' || r != &s) abort(); r->i = 17; r->j = 23; if (s.i != 17 || s.j != 23) abort(); return 0; }
