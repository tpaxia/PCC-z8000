/* A constant cast to a structure pointer keeps the structure type, also when
 * the constant does not fit in an int, as in the V7 kernel's definition of u
 * as (*(struct user *)0xF000). Losing it gave "illegal member use" warnings
 * and "gummy structure" compiler errors that depended on what else the file
 * declared, so the declarations below are reduced from the kernel's main.c.
 * Only member addresses are formed; nothing at those addresses is touched. */
typedef	struct { int r[1]; } *	physadr;
typedef	long		daddr_t;
typedef	int		label_t[12];
struct proc *runq;
struct buf *breada();
struct filsys *getfs();
struct	direct
{
	char	d_name[14];
};
struct	user
{
	struct proc *u_procp;
	struct {
		unsigned ux_drsize;
	} u_exdata;
};
struct	filsys {
	daddr_t	s_free[50];
} mount[2];
struct	proc {
	short	p_addr;
} linesw[1];
struct buf
{
	struct	buf *b_forw;
};
struct proc proc[8];
unsigned where(p) char *p; { return (unsigned)p; }
struct proc **slot() { return &(*(struct user *)0xF000).u_procp; }
main()
{
	if (where((char *)&(*(struct user *)0xF000)) != 0xF000) return 1;
	if (where((char *)slot()) != 0xF000) return 2;
	if (where((char *)&(*(struct user *)0xF000).u_exdata.ux_drsize) != 0xF002) return 3;
	if (where((char *)&((struct user *)0xF000)->u_exdata) != 0xF002) return 4;
	if (where((char *)&(*(struct user *)0x100).u_exdata) != 0x102) return 5;
	if (where((char *)&((struct filsys *)0x8000L)->s_free[2]) != 0x8008) return 6;
	return 0;
}
