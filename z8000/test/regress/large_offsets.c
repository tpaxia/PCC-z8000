struct large { char data[9000]; int tail; };
static struct large global;
struct link_a { struct link_a *next; int value; };
struct link_b { struct link_b *next; long value; };

check(n)
int n;
{
    char frame[10000];
    int i;
    for (i=0; i<10000; i+=113) frame[i] = i&63;
    {
        char inner[5000];
        inner[4999] = n;
        if (inner[4999] != n) return 1;
    }
    for (i=0; i<10000; i+=113) if (frame[i] != (i&63)) return 2;
    return 0;
}

main()
{
    struct large local;
    struct link_a a;
    struct link_b b;
    a.next = &a;
    b.next = &b;
    a.value = 17;
    b.value = 123456L;
    if (a.next != &a || b.next != &b || a.value != 17 ||
        b.value != 123456L) return 6;
    local.data[8999] = 37;
    local.tail = 1234;
    global.tail = local.tail;
    if (sizeof(local) != 9002 || global.tail != 1234) return 3;
    if (check(42)) return 4;
    if (local.data[8999] != 37 || local.tail != 1234) return 5;
    return 0;
}
