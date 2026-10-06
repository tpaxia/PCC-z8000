/* Native optimizer for current PCC Z8002 C assembly (shared csv/cret frames).
 * Disk-backed passes keep assembly size independent of the 64K data space.
 * The bounded jump map is optional: a full map only loses optimizations.
 */
#include <stdio.h>
#include <signal.h>
#ifndef z8000
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#else
char *malloc(), *strcpy();
#endif

#define LINE 512
#define ARG 128
#define HASH 257
#define MAPMAX 24000
struct target { struct target *next; char *from, *to; };
struct target *htable[HASH];
int mapused, changed;
char names[2][32], op[16], one[ARG], two[ARG];
int nargs;

cleanup()
{
    unlink(names[0]); unlink(names[1]);
}

fatal(s)
char *s;
{
    fprintf(stderr, "oz8: %s\n", s);
    cleanup(); exit(1);
}

#ifndef z8000
void
#endif
caught(sig)
int sig;
{
    cleanup(); exit(1);
}

space(c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

char *trim(s)
char *s;
{
    char *end;
    while (space(*s)) s++;
    end = s + strlen(s);
    while (end > s && space(end[-1])) --end;
    *end = 0;
    return s;
}

readln(f, s)
FILE *f;
char *s;
{
    int n;
    if (!fgets(s, LINE, f)) {
        if (ferror(f)) fatal("input read failed");
        return 0;
    }
    n = strlen(s);
    if (n && s[n-1] == '\n') s[n-1] = 0;
    else if (!feof(f)) fatal("assembly line too long");
    return 1;
}

putln(f, s)
FILE *f;
char *s;
{
    fputs(s, f); putc('\n', f);
    if (ferror(f)) fatal("output write failed");
}

FILE *openf(i, mode)
int i;
char *mode;
{
    FILE *f;
    f = fopen(names[i], mode);
    if (!f) fatal("cannot open temporary file");
    return f;
}

closef(f)
FILE *f;
{
    if (fclose(f)) fatal("temporary file close failed");
}

/* Parse only simple assembly lines; unrecognized input is retained. */
parse(s)
char *s;
{
    char buf[LINE], *p, *start;
    int n;
    op[0] = one[0] = two[0] = 0; nargs = 0;
    strcpy(buf, s); p = trim(buf); start = p;
    while (*p && !space(*p)) p++;
    n = p - start;
    if (n >= sizeof op) return;
    if (*p) *p++ = 0;
    strcpy(op, start); p = trim(p);
    if (!*p) return;
    start = p;
    while (*p && *p != ',') p++;
    if (*p) *p++ = 0;
    start = trim(start);
    if (strlen(start) >= ARG || strlen(p) >= ARG) { op[0]=0; return; }
    strcpy(one, start); nargs = 1;
    if (*p) { strcpy(two, trim(p)); nargs = 2; }
}

local(s)
char *s;
{
    if (*s++ != '.' || *s++ != 'L' || *s < '0' || *s > '9') return 0;
    while (*s >= '0' && *s <= '9') s++;
    return *s == 0;
}

label(s, dst)
char *s, *dst;
{
    char buf[LINE], *p;
    int n;
    strcpy(buf, s); p = trim(buf); n = strlen(p);
    if (n < 2 || n >= ARG || p[n-1] != ':') return 0;
    p[n-1] = 0;
    if (!local(p)) return 0;
    strcpy(dst, p); return 1;
}

jump() { return !strcmp(op,"jr") || !strcmp(op,"jp"); }

symbol(s)
char *s;
{
    if (*s == '.') s++;
    if (!((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || *s == '_')) return 0;
    while (*s) {
        if (!((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') ||
              (*s >= '0' && *s <= '9') || *s == '_' || *s == '.')) return 0;
        s++;
    }
    return 1;
}

hash(s)
char *s;
{
    unsigned h;
    h = 0;
    while (*s) h = (h * 33 + *s++) & 32767;
    return h % HASH;
}

freemap()
{
    int i;
    struct target *p, *next;
    for (i=0; i<HASH; i++) {
        for (p=htable[i]; p; p=next) { next=p->next; free(p); }
        htable[i]=0;
    }
    mapused=0;
}

char *lookup(s)
char *s;
{
    struct target *p;
    for (p=htable[hash(s)]; p; p=p->next)
        if (!strcmp(s,p->from)) return p->to;
    return s;
}

enter(from, to)
char *from, *to;
{
    struct target *p;
    int n, h, cost;
    n = sizeof(*p) + strlen(from) + strlen(to) + 2;
    /* Fixed accounting gives host and target the same saturation point. */
    cost = 32 + strlen(from) + strlen(to) + 2;
    if (mapused + cost > MAPMAX) return;
    p = (struct target *)malloc(n);
    if (!p) return;
    mapused += cost;
    p->from = (char *)(p+1); strcpy(p->from,from);
    p->to = p->from + strlen(from) + 1; strcpy(p->to,to);
    h=hash(from); p->next=htable[h]; htable[h]=p;
}

deadcode()
{
    FILE *in, *out;
    char line[LINE], copy[LINE], *p;
    int dead, n;
    in=openf(0,"r"); out=openf(1,"w"); dead=0;
    while (readln(in,line)) {
        strcpy(copy,line); p=trim(copy); n=strlen(p);
        if (!n || *p=='!') { putln(out,line); continue; }
        if (p[n-1]==':' || *p=='.' || indexeq(p)) dead=0;
        else if (dead) { changed=1; continue; }
        putln(out,line); parse(line);
        if ((jump() && nargs==1 && symbol(one)) || !strcmp(p,"ret")) dead=1;
    }
    closef(in); closef(out);
}

indexeq(s)
char *s;
{
    while (*s) if (*s++ == '=') return 1;
    return 0;
}

makemap()
{
    FILE *in;
    char line[LINE], copy[LINE], pending[ARG], *p;
    in=openf(1,"r"); pending[0]=0; freemap();
    while (readln(in,line)) {
        strcpy(copy,line); p=trim(copy);
        if (!*p || *p=='!') continue;
        parse(line);
        if (pending[0] && jump() && nargs==1 && local(one)) enter(pending,one);
        if (!label(line,pending)) pending[0]=0;
    }
    closef(in);
}

/* Look ahead over labels/comments without buffering a whole basic block. */
nextlab(in, target)
FILE *in;
char *target;
{
    long pos;
    char line[LINE], *p, lab[ARG];
    int yes, n;
    pos=ftell(in); yes=0;
    if (pos < 0) fatal("temporary file position failed");
    while (readln(in,line)) {
        p=trim(line); n=strlen(p);
        if (!n || *p=='!') continue;
        if (p[n-1]!=':') break;
        if (label(p,lab) && !strcmp(lab,target)) { yes=1; break; }
    }
    if (fseek(in,pos,0)) fatal("temporary file seek failed");
    return yes;
}

branches()
{
    FILE *in, *out;
    char line[LINE], *target, *tail;
    int n;
    in=openf(1,"r"); out=openf(0,"w");
    while (readln(in,line)) {
        parse(line);
        if (jump() && nargs) {
            target=nargs==1 ? one : two;
            if (local(target)) {
                tail=lookup(target);
                if (strcmp(target,tail)) {
                    n=strlen(line);
                    while (n && space(line[n-1])) --n;
                    n-=strlen(target);
                    if (n+strlen(tail) >= LINE) fatal("branch line too long");
                    strcpy(line+n,tail); changed=1; parse(line);
                }
            }
        }
        if ((jump() && nargs==1 && local(one) && nextlab(in,one)) ||
            (!strcmp(op,"ld") && nargs==2 && regnum(one)>=0 && !strcmp(one,two))) {
            changed=1; continue;
        }
        putln(out,line);
    }
    closef(in); closef(out); freemap();
}

regnum(s)
char *s;
{
    int n;
    if (*s++!='r' || *s<'0' || *s>'9') return -1;
    n=0;
    while (*s>='0' && *s<='9') { n=n*10+*s++-'0'; if(n>15) return -1; }
    return *s ? -1 : n;
}

/* Compiler frame displacements fit a signed word. */
memarg(s, off, base)
char *s;
int *off, *base;
{
    int sign, n;
    sign=1; n=0;
    if (*s=='-') { sign = -1; s++; }
    if (*s<'0' || *s>'9') return 0;
    while (*s>='0' && *s<='9') {
        if (n>3276 || (n==3276 && *s>'7')) return 0;
        n=n*10+*s++-'0';
    }
    if (*s++!='(' || *s++!='r') return 0;
    *off=n*sign; n=0;
    if (*s<'0' || *s>'9') return 0;
    while (*s>='0' && *s<='9') { n=n*10+*s++-'0'; if(n>15) return 0; }
    if (*s++!=')' || *s) return 0;
    *base=n; return 1;
}

shorten()
{
    FILE *in, *out;
    char line[LINE];
    int n;
    in=openf(0,"r"); out=openf(1,"w");
    while(readln(in,line)) {
        parse(line);
        if (!strcmp(op,"ld") && regnum(one)>=0 && !strcmp(two,"#0"))
            sprintf(line,"\tclr\t%s",one);
        if ((!strcmp(op,"add") || !strcmp(op,"sub")) && !strcmp(one,"sp") && two[0]=='#') {
            n=atoi(two+1);
            if (n>=1 && n<=16) {
                char num[8];
                sprintf(num,"#%d",n);
                if (!strcmp(two,num)) sprintf(line,"\t%s\tsp,#%d",!strcmp(op,"add")?"inc":"dec",n);
            }
        }
        putln(out,line);
    }
    closef(in); closef(out);
}

pairval(kind, r, v, b)
int kind, *r, *v, *b;
{
    if (strcmp(op,"ld") || nargs!=2) return 0;
    if (kind==2) { *r=regnum(two); return *r>=0 && memarg(one,v,b); }
    *r=regnum(one);
    if (*r<0) return 0;
    if (kind==0) return memarg(two,v,b);
    *v=regnum(two); *b=0; return *v>=0;
}

pairs(kind, src, dst)
int kind, src, dst;
{
    FILE *in, *out;
    char line[LINE], next[LINE];
    int a,b,c,d,e,f,ok;
    long pos;
    in=openf(src,"r"); out=openf(dst,"w");
    while (readln(in,line)) {
        parse(line);
        if (!pairval(kind,&a,&b,&c)) { putln(out,line); continue; }
        pos=ftell(in);
        if (pos<0) fatal("temporary file position failed");
        if (!readln(in,next)) { putln(out,line); break; }
        parse(next);
        if (!pairval(kind,&d,&e,&f)) {
            if (fseek(in,pos,0)) fatal("temporary file seek failed");
            putln(out,line); continue;
        }
        ok=!(a&1) && d==a+1;
        if (kind==1) ok=ok && !(b&1) && e==b+1;
        else ok=ok && b<=32765 && e==b+2 && c==f && (kind!=0 || c!=a);
        if (ok) {
            if (kind==0) sprintf(line,"\tldl\trr%d,%d(r%d)",a,b,c);
            if (kind==1) sprintf(line,"\tldl\trr%d,rr%d",a,b);
            if (kind==2) sprintf(line,"\tldl\t%d(r%d),rr%d",b,c,a);
            putln(out,line);
        } else { putln(out,line); putln(out,next); }
    }
    closef(in); closef(out);
}

main()
{
    FILE *in, *out;
    char line[LINE];
    int enabled, turn;
    sprintf(names[0],"/tmp/oz%da",getpid());
    sprintf(names[1],"/tmp/oz%db",getpid());
    if (signal(SIGINT,SIG_IGN)!=SIG_IGN) signal(SIGINT,caught);
    if (signal(SIGTERM,SIG_IGN)!=SIG_IGN) signal(SIGTERM,caught);
    enabled=0; out=openf(0,"w");
    while(readln(stdin,line)) {
        parse(line);
        if (!strcmp(op,"call") && !strcmp(one,"csv") && nargs==1) enabled=1;
        putln(out,line);
    }
    closef(out);
    if (enabled) {
        for (turn=0;turn<8;turn++) {
            changed=0; deadcode(); makemap(); branches();
            if (!changed) break;
        }
        shorten(); pairs(0,1,0); pairs(1,0,1); pairs(2,1,0);
    }
    in=openf(0,"r");
    while(readln(in,line)) putln(stdout,line);
    closef(in);
    if (fflush(stdout)) fatal("stdout write failed");
    cleanup(); return 0;
}
