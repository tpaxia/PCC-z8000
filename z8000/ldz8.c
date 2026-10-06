#include <stdio.h>
#include <ar.h>
#include "b.out.h"


long	atol();
char	*strcat();
char	*strcpy();


/*
 *  link editor for Z8000
 */

#define NOVLY	16
#define	NROUT	256
#define	NSYM	4003
#define	NSYMPR	4000
#define TABSZ	700


typedef struct symbol *symp;
struct symbol
{
	struct sym s;			/* type and value */
	char *sname;			/* pointer to asciz name */
	int snlength;			/* length of name in bytes */
	symp *shash;			/* hash table index */
	unsigned sindex;		/* id of symbol in symtab */
};

typedef struct arg_link *arg;
struct arg_link
{
	char *arg_name;			/* the acutal argument string */
	arg arg_next;			/* next one in linked list */
};

typedef struct arc_entry *arce;
struct arc_entry
{
	long	arc_offs;		/* offset in the file of this entry */
	arce	arc_e_next;		/* next entry, NULL if none */
};

typedef struct arc_link *arcp;
struct arc_link
{
	arce arc_e_list;		/* list of archive entries */
	arcp arc_next;			/* next archive */
};
/* global variables */

arg arglist;				/* linked list of arguments */
struct ar_hdr archdr;			/* directory part of archive */
arcp arclist = NULL;			/* list of archives */
arcp arclast = NULL;			/* last archive on arclist */
arce arcelast = NULL;			/* last entry in this entry list */
struct bhdr filhdr;			/* header file for current file */
struct symbol cursym;			/* current symbol */
char csymbuf[SYMLENGTH];		/* buffer for current symbol name */
struct symbol symtab[NSYM];		/* actual symbols */
symp lastsym;				/* last symbol entered */
int symindex;				/* next available symbol table entry */
symp hshtab[NSYM+2];			/* hash table for symbols */
symp local[NSYMPR];			/* symbols in current file */
int nloc;				/* number of local symbols per file */
int nund = 0;				/* number of undefined syms in pass 2*/
symp entrypt;				/* pointer to entry point symbol */
int argnum;				/* current argument number */
char ldz8[] = "ldz8 -- link editor for Z8000";
char *ofilename = "a.out";		/* name of output file, default init */
FILE *text;				/* file descriptor for input file */
FILE *rtext;				/* used to access relocation */
char *filename;				/* name of current input file */
char libname[64];			/* name of current library */
#ifdef z8000
char libdir[64] = "/lib/lib";			/* prefix for -l libraries */
#else
char libdir[64] = "/projects/nunix/lib/lib";	/* prefix for -l libraries */
#endif
FILE *tout;				/* text portion */
FILE *dout;				/* data portion */
FILE *trout;				/* text relocation commands */
FILE *drout;				/* data relocation commands */


/* flags */
int	xflag;		/* discard local symbols */
int	Xflag;		/* discard locals starting with '.L' */
int	Sflag;		/* discard all except locals and globals*/
int	rflag;		/* preserve relocation bits, don't define common */
int	sflag;		/* discard all symbols */
int	dflag;		/* define common even with rflag */
int	iflag;		/* separate I/D, 0411 */
int	nflag = 0;	/* create a 410 file */

/* used after pass 1 */
long torigin;		/* origin of text segment in final output */
long dorigin;		/* origin of data segment in final output */
long doffset;		/* current position in output data segment */
long borigin;		/* origin of bss segment in final output */
long corigin;		/* origin of common area */

/* cumulative sizes set in pass 1 */
long	tsize;		/* size of text data */
long	dsize;		/* size of data segment */
long	bsize;		/* size of bss segment */
long	csize;		/* size of common area */
long	rtsize;		/* size of text relocation area */
long	rdsize;		/* size of data relocation area */
long	ssize = 0;	/* size of symbol area */

/* symbol relocation; both passes */
long	ctrel;
long	cdrel;
long	cbrel;

/* error messages */

char *e1 = "missing argument following command option -%c";
char *e2 = "unrecognized option: -%c";
char *e3 = "premature end of file %s";
char *e4 = "read error on file %s";
char *e5 = "unknown type of file %s";
char *e6 = "multiply defined symbol %s in file %s";
char *e7 = "entry point not in text";
char *e8 = "cannot create file %s";
char *e9 = "cannot reopen output file";
char *e10 = "file symbol overflow";
char *e11 = "internal error - undefined symbol %s in file %s";
char *e12 = "format error in file %s, bad relocation size";
char *e13 = "format error in file %s, relocation outside of segment";
char *e14 = "error writing file %s";
char *e15 = "file %s not found";
char *e16 = "hash table overflow";
char *e17 = "symbol table overflow";
char *e18 = "format error in file %s, null symbol name";
char *e19 = "format error in file %s, invalid symbol id in relocation command";
char *e20 = "format error in file %s, bad type of symbol %s in reloc command";
char *e21 = "format error in file %s, bad address in relocation command";


/* functions */

long getsym();
symp lookup();
symp slookup();
symp *hash();
long relext();
long relcmd();
long atox();
int nerrors;	/* number of errors reported by error() */
/* main -	The link editor works in two passes.  In pass 1, symbols
		are defined.  In pass 2, the actual text and data will
		be output along with any relocation commands and symbols
*/

main(argc, argv)
int argc;
char **argv;
{
	arg argp;	/* current argument */

	torigin = 0;	/* set with -R flag */
	procargs(argc, argv);
	for (argp = arglist; argp; argp = argp->arg_next)
		load1arg(filename = argp->arg_name);
	middle();
	setupout();
	for (argp = arglist; argp; argp = argp->arg_next)
		load2arg(filename = argp->arg_name);
	finishout();
	/* An image is still written, but a link with undefined or multiply
	   defined symbols or bad relocation commands is not a success. */
	exit(nerrors || nund ? 1 : 0);
}

/* progargs -	Process command arguments
 */

procargs(argc, argv)
int argc;
char **argv;
{
	for (argnum = 1; argnum < argc; argnum++)	/* for each arg */
	{
		if (argv[argnum][0] == '-') procflags(argc, argv);
		else newarg(argv[argnum]);	/* build linked list */
	}
}


/* newarg -	Create a new member of linked list of arguments
 */
newarg(name)
char *name;
{
	arg a1, a2;
	a1 = (arg)calloc(1, sizeof(*a1));
	a1->arg_name = name;
	a1->arg_next = NULL;
	if (arglist == NULL) arglist = a1;
	else	/* link new one on end */
	{
		for (a2 = arglist; a2->arg_next; a2 = a2->arg_next);
		a2->arg_next = a1;
	}
}
/* procflags -	Process flag arguments.
 */

procflags(argc, argv)
int argc;
char **argv;
{
	char *flagp = &argv[argnum][1];
	char c;

	while(c = *flagp++) switch(c)
	{
	case 'o':
		if (++argnum >= argc) error(e1, c);
		else ofilename = argv[argnum];
		break;
	case 'L':
		if (++argnum >= argc) error(e1, c);
		else strcpy(libdir, argv[argnum]);
		break;
	case 'u':
		if (++argnum >= argc) error(e1, c);
		else enter(slookup(argv[argnum]));
		break;
	case 'e':
		if (++argnum >= argc) error(e1, c);
		else enter(slookup(argv[argnum]));
		entrypt = lastsym;
		error("-e is currently ignored");
		break;
	case 'D':
		if (++argnum >= argc) error(e1, c);
		else dsize = atol(argv[argnum]);
		break;
	case 'R':
		if (++argnum >= argc) error(e1, c);
		else torigin = atox(argv[argnum]);
		break;
	case 'l': newarg(argv[argnum]); return;
	case 'x': xflag++; break;
	case 'X': Xflag++; break;
	case 'S': Sflag++; break;
	case 'r': rflag++; break;
	case 's': sflag++; xflag++; break;
	case 'd': dflag++; break;
	case 'n': nflag++; break;
	case 'i': iflag++; break;
	default:	error(e2, c);
	}
}

long atox(s)
char *s;
{
	long result = 0;
	for (; *s != 0; *s++)
	  if (*s >= '0' && *s <= '9') result = (result<<4) + *s-'0'; else
	  if (*s >= 'a' && *s <= 'f') result = (result<<4) + *s-'a'+10; else
	  if (*s >= 'A' && *s <= 'F') result = (result<<4) + *s-'A'+10; else
	  error("atox: illegal hex argument (%c)",*s);
	return(result);
}
/* scan file to find defined symbols */
load1arg(cp)
register char *cp;
{
	long position = 0;
	arcp ap;		/* pointer to new archive list element */

	switch (getfile(cp))	/* open file and read first word */
	{
	case FMAGIC:
		fseek(text, 0L, 0);	/* back to start of file */
		load1(0L, 0);		/* pass1 on regular file */
		break;

	/* regular archive */
	case ARCMAGIC:
		ap = (arcp)calloc(1, sizeof(*ap));
		ap->arc_next = NULL;
		ap->arc_e_list = NULL;
		if (arclist)
		{
			arclast->arc_next = ap;
			arclast = ap;
		}
		else arclast = arclist = ap;
		position = SARMAG;
		fseek(text, (long)SARMAG, 0);	/* skip magic word */
/**/		while (fread(&archdr, sizeof archdr, 1, text) && !feof(text))
		{
			if (strncmp(archdr.ar_fmag,ARFMAG,strlen(ARFMAG)) != 0)
			  fatal("error in archive format: %s",filename);
			position += sizeof(archdr);
			if (load1(position, 1))		/* arc pass1 */
			{
				arce ae = (arce)calloc(1, sizeof(*ae));
				ae->arc_e_next = NULL;
				ae->arc_offs = position;
				if (arclast->arc_e_list)
				{
					arcelast->arc_e_next = ae;
					arcelast = ae;
				}
				else arclast->arc_e_list = arcelast = ae;
			}
			position += (atol(archdr.ar_size) + 1)&~1;
			fseek(text, position, 0);
		}
		break;

	default:
		fatal(e5, filename);
	}
	fclose(text);
	fclose(rtext);
}
/* load1 -	Accumulate file sizes and symbols for single file
		or archive member.  Relocate each symbol as it is
		processed to its relative position in it csect.
		If libflg == 1, then file is an archive hence
		throw it away unless it defines some symbols.
		Here we also accumulate .comm symbol values.
*/

load1(sloc, libflg)
long sloc;	/* location of first byte of this file */
int libflg;	/* 1 => loading a library, 0 else */
{
	long loc = sloc;/* current position in file */
	long endpos;	/* position after symbol table */
	int savindex;	/* symbol table index on entry */
	int ndef;	/* number of symbols defined */

	readhdr(sloc);

	ctrel = tsize;
	cdrel += dsize;
	cbrel += bsize;
	ndef = 0;
	nloc = 0;
	savindex = symindex;
	loc += SYMPOS;
	fseek(text,loc, 0);	/* skip to symbols */
	endpos = loc + filhdr.ssize;
	while (loc < endpos)
	{
		loc += getsym();
		ndef += sym1();	/* process one symbol */
	}
	if (libflg==0 || ndef) {
		tsize += filhdr.tsize;
		dsize += filhdr.dsize;
		bsize += filhdr.bsize;
		ssize += nloc;			/* count local symbols */
		rtsize += filhdr.trsize;
		rdsize += filhdr.drsize;
		return(1);
	}
	/*
	 * No symbols defined by this library member.
	 * Rip out the hash table entries and reset the symbol table.
	 */
	while (symindex>savindex) *symtab[--symindex].shash = 0;
	return(0);
}
/* sym1 -	Process pass1 symbol definitions.  This involves flushing
		un-needed symbols, and entering the rest in the symbol table.
		Returns 1 if a symbol was defined and 0 else.
 */
sym1()
{
	register symp sp;
	int type = cursym.s.stype;	/* type of the current symbol */

	if (Sflag) switch(type & ~EXTERN)
	{
	case TEXT:
	case BSS:
	case DATA:
		break;
	default:
		return(0);
	}
	if ((type&EXTERN) == 0)
	{
		if (xflag || (Xflag && (cursym.sname[0] == '.')
			&& (cursym.sname[1] == 'L')));
		else nloc += NLIST_DISKSIZE;
		return(0);
	}
	symreloc(); 			/* relocate symbol in file */
	if (enter(lookup())) return(0);	/* install symbol in table */

	/* If we get here, then symbol was already present.
	   If symbol is already defined then return, multiple
	   definitions are caught later.   If current symbol is
	   not EXTERN eg it is local, let the new one override */

	if ((sp = lastsym)->s.stype & EXTERN)
	{
		if ((sp->s.stype & ~EXTERN) != UNDEF) return(0);
	}
	else if (!(cursym.s.stype & EXTERN)) return(0);
	if (cursym.s.stype == EXTERN+UNDEF)
	{
		/* check for .comm symbols */
		if (cursym.s.svalue>sp->s.svalue) sp->s.svalue=cursym.s.svalue;
		return(0);
	}
	if (sp->s.svalue!=0 && cursym.s.stype == EXTERN+TEXT) return(0);
	sp->s.stype = cursym.s.stype;	/* define something new */
	sp->s.svalue = cursym.s.svalue;
	return(1);
}
/* middle -	Finalize the symbol table by adding the origins of
		each csect to symbols.  Also define the end stuff,
		adjusting the boundries of the csect depending on
		options.
 */

middle()
{
	register symp sp;

	enter(slookup("_etext"));
	enter(slookup("_edata"));
	enter(slookup("_end"));
	/*
	 * Assign common locations.
	 */
	csize = 0;
	if (dflag || rflag==0)
	{
		ldrsym(slookup("_etext"), tsize, EXTERN+TEXT);	/* set value */
		ldrsym(slookup("_edata"), dsize, EXTERN+DATA);
		ldrsym(slookup("_end"), bsize, EXTERN+BSS);
		common();
	}
	/*
	 * Now set symbols to their final value
	 */
	tsize = (tsize + 1) & ~01;		/* move to word boundry */
	if (iflag) dorigin = 0;
	else if (nflag) dorigin = torigin+(tsize+(PAGESIZE - 1)) & ~(PAGESIZE - 1);
	else dorigin = torigin + tsize;
	dsize = (dsize + 1) & ~01;		/* move to word boundry */
	corigin = dorigin + dsize;		/* common after data */
	borigin = corigin + csize;		/* bss after common area */
	fprintf(stderr, "DEBUG middle: torigin=0x%lx tsize=0x%lx dorigin=0x%lx dsize=0x%lx corigin=0x%lx csize=0x%lx borigin=0x%lx\n",
		torigin, tsize, dorigin, dsize, corigin, csize, borigin);
	nund = 0;				/* no undefined initially */
/*	ssize = size of local symbols to be entered in load2		*/
	doffset = 0;				/* beginning of data seg */
	for (sp = symtab; sp < &symtab[symindex]; sp++)  sym2(sp);
	bsize += csize;
	/* Check before 16-bit a.out fields and relocations can truncate sizes. */
	if (iflag && (rflag || nflag || torigin != 0))
		fatal("-i requires origin zero and cannot be combined with -r or -n");
	if (rflag == 0 && (tsize > 65535L || dsize > 65535L || bsize > 65535L ||
	    (iflag ? dsize+bsize > 65536L : dorigin+dsize+bsize > 65536L)))
		fatal("executable exceeds 16-bit address space");
}
/* common -	Set up the common area.
 */

common()
{
	register symp sp;
	long val;

	csize = 0;
	for (sp = symtab; sp < &symtab[symindex]; sp++)
		if (sp->s.stype == EXTERN+UNDEF && ((val = sp->s.svalue) != 0))
		{
			val = (val + 1) & ~01;	/* word boundry */
			if (sp->sname && (strcmp(sp->sname, "buf") == 0 || strcmp(sp->sname, "bfreelis") == 0 || strcmp(sp->sname, "buffers") == 0))
				fprintf(stderr, "DEBUG common: %s size=%ld offset=%ld\n", sp->sname, val, csize);
			sp->s.svalue = csize;
			sp->s.stype = EXTERN+COMM;
			csize += val;
		}
	fprintf(stderr, "DEBUG common: total csize=%ld\n", csize);
}
/* sym2 -	Assign external symbols their final value and compute ssize */
sym2(sp)
register symp sp;
{
	ssize += NLIST_DISKSIZE;
	switch (sp->s.stype)
	{
	case EXTERN+UNDEF:
		if ((rflag == 0 || dflag) && sp->s.svalue == 0)
		{
			if (nund==0)
				printf("ldz8: Undefined -\n");
			nund++;
			printf("\t%s\n", sp->sname);
		}
		break;

	case EXTERN+ABS:
	default:
		break;

	case EXTERN+TEXT:
		sp->s.svalue += torigin;
		break;

	case EXTERN+DATA:
		if (sp->sname && strcmp(sp->sname, "bdevsw") == 0)
			fprintf(stderr, "DEBUG sym2: bdevsw before=0x%lx dorigin=0x%lx\n", sp->s.svalue, dorigin);
		sp->s.svalue += dorigin;
		if (sp->sname && strcmp(sp->sname, "bdevsw") == 0)
			fprintf(stderr, "DEBUG sym2: bdevsw after=0x%lx\n", sp->s.svalue);
		break;

	case EXTERN+BSS:
		sp->s.svalue += borigin;
		break;

	case EXTERN+COMM:
		sp->s.stype = EXTERN+BSS;
		if (sp->sname && (strcmp(sp->sname, "buf") == 0 || strcmp(sp->sname, "bfreelis") == 0 || strcmp(sp->sname, "buffers") == 0))
			fprintf(stderr, "DEBUG sym2 COMM: %s before=0x%lx corigin=0x%lx\n", sp->sname, sp->s.svalue, corigin);
		sp->s.svalue += corigin;
		if (sp->sname && (strcmp(sp->sname, "buf") == 0 || strcmp(sp->sname, "bfreelis") == 0 || strcmp(sp->sname, "buffers") == 0))
			fprintf(stderr, "DEBUG sym2 COMM: %s after=0x%lx\n", sp->sname, sp->s.svalue);
		break;
	}
}
/* ldrsym -	Force the definition of a symbol */
ldrsym(asp, val, type)
symp asp;
long val;			/* value of the symbol */
int  type;			/* its type */
{
	register symp sp;

	if ((sp = asp) == 0)
		return;
	if (sp->s.stype != EXTERN+UNDEF || sp->s.svalue)
	{
		error(e6, sp->sname, filename);
		return;
	}
	sp->s.stype = type;
	sp->s.svalue = val;
}

/* setupout -	Set up for output.  Create output file and a temporary
		files for the data area and relocation commands if
		necessary.  Write the header on the output file.
 */

setupout()
{
	if (iflag) filhdr.fmagic = IMAGIC;
	else if (nflag) filhdr.fmagic = NMAGIC;
	else filhdr.fmagic = FMAGIC;
	filhdr.tsize = tsize;
	filhdr.dsize = dsize;
	filhdr.bsize = bsize;
	filhdr.trsize = rflag? rtsize: 0;
	filhdr.drsize = rflag? rdsize: 0;
	filhdr.ssize = sflag? 0:ssize;
	if (entrypt)
	{
		if (entrypt->s.stype!=EXTERN+TEXT) error(e7);
		else filhdr.entry = entrypt->s.svalue;
	}
	else filhdr.entry = 0;
	filhdr.entry = torigin;
	if ((tout = fopen(ofilename, "w")) == NULL) fatal(e8, ofilename);
	if ((dout = fopen(ofilename, "r+")) == NULL) fatal(e9);
	fseek(dout, (long)(DATAPOS), 0);
	if (rflag)
	{
		if ((trout = fopen(ofilename, "r+")) == NULL) fatal(e9);
		fseek(trout, (long)RTEXTPOS, 0); /* start of text relocation */
		if ((drout = fopen(ofilename, "r+")) == NULL) fatal(e9);
		fseek(drout, (long)RDATAPOS, 0);	/* to data reloc */
	}
	put68(tout, &filhdr.fmagic, 2);
	put68(tout, &filhdr.tsize, 2);
	put68(tout, &filhdr.dsize, 2);
	put68(tout, &filhdr.bsize, 2);
	put68(tout, &filhdr.ssize, 2);
	put68(tout, &filhdr.entry, 2);
	put68(tout, &filhdr.trsize, 2);
	put68(tout, &filhdr.drsize, 2);
}
/* load2arg -	Load a named file or an archive */

load2arg(cp)
char *cp;
{
	long position = 0;	/* position in input file */
	arce entry;		/* pointer to current entry */
	switch (getfile(cp))
	{
	case FMAGIC:			/* normal file */
	{
		unsigned short tmp;
		get68(text, &tmp, 2); filhdr.fmagic = tmp;
		get68(text, &tmp, 2); filhdr.tsize = tmp;
		get68(text, &tmp, 2); filhdr.dsize = tmp;
		get68(text, &tmp, 2); filhdr.bsize = tmp;
		get68(text, &tmp, 2); filhdr.ssize = tmp;
		get68(text, &tmp, 2); filhdr.entry = tmp;
		get68(text, &tmp, 2); filhdr.trsize = tmp;
		get68(text, &tmp, 2); filhdr.drsize = tmp;
		load2(0L);
		break;
	}
	case ARCMAGIC:			/* archive */
		for(entry=arclist->arc_e_list; entry; entry=entry->arc_e_next)
		{
			unsigned short tmp;
			position = entry->arc_offs;
			fseek(text, position, 0);
			get68(text, &tmp, 2); filhdr.fmagic = tmp;
			get68(text, &tmp, 2); filhdr.tsize = tmp;
			get68(text, &tmp, 2); filhdr.dsize = tmp;
			get68(text, &tmp, 2); filhdr.bsize = tmp;
			get68(text, &tmp, 2); filhdr.ssize = tmp;
			get68(text, &tmp, 2); filhdr.entry = tmp;
			get68(text, &tmp, 2); filhdr.trsize = tmp;
			get68(text, &tmp, 2); filhdr.drsize = tmp;
			load2(position);		/* load the file */
		}
		arclist = arclist->arc_next;
		break;
	default:	bletch("bad file type on second pass");
	}
	fclose(text);
	fclose(rtext);
}

/* load2 -	Actually output the text, performing relocation as necessary.
 */
load2(sloc)
long sloc;	/* position of filhdr in current input file */
{
	register symp sp;
	long loc=sloc;	/* current position in file */
	long endpos;	/* end of symbol segment */
	int symno = 0;
	int type;

	readhdr(sloc);
	ctrel = torigin;
	cdrel += dorigin;
	cbrel += borigin;

	/*
	 * Reread the symbol table, recording the numbering
	 * of symbols for fixing external references.
	 */
	loc += SYMPOS;
	fseek(text, loc, 0);
	endpos = loc + filhdr.ssize;
	while(loc < endpos)
	{
		loc += getsym();
		if (++symno >= NSYMPR) fatal(e10);
		symreloc();
		type = cursym.s.stype;
		if (Sflag)
		{
			switch(type & ~EXTERN)
			{
			case TEXT:
			case BSS:
			case DATA:
				break;
			default:
				continue;
			}
		}
		if ((type&EXTERN) == 0)		/* enter local symbols now */
		{
			if (xflag || (Xflag && (cursym.sname[0] == '.')
				&& (cursym.sname[1] == 'L')));
			else enter(lookup());
			continue;
		}
		if ((sp = lookup()) == NULL) fatal(e11,cursym.sname,filename);
		local[symno - 1] = sp;
		if (cursym.s.stype != EXTERN+UNDEF
		 && (cursym.s.stype != sp->s.stype
		  || cursym.s.svalue != sp->s.svalue))
			error(e6, cursym.sname, filename);
	}
	fseek(text, sloc+TEXTPOS, 0);
	fseek(rtext, sloc+RTEXTPOS, 0);
	load2td(tout, trout, torigin, filhdr.tsize, filhdr.trsize);
	fseek(text, sloc+DATAPOS, 0);
	fseek(rtext, sloc+RDATAPOS, 0);
	load2td(dout, drout, doffset, filhdr.dsize, filhdr.drsize);
	torigin += filhdr.tsize;
	dorigin += filhdr.dsize;
	doffset += filhdr.dsize;
	borigin += filhdr.bsize;
}
/* load the text or data section of a file performing relocation */
load2td(outf, outrf, txtstart, txtsize, rsize)
FILE *outf;		/* text or data portion of output file */
FILE *outrf;		/* text or data relocation part of output file */
long txtstart;		/* initial offset of text or data segment */
long txtsize;		/* number of bytes in segment */
long rsize;		/* size of appropriate relocation data */
{
	struct reloc rel;		/* the current relocation command */
	int size;			/* number of bytes to relocate */
	long offs;			/* value of offset to use */
	long rcount;			/* number of relocation commands */
	long pos = 0;			/* current input position */

	rcount = rsize/RELOC_DISKSIZE;
	while(rcount--)			/* for each relocation command */
	{
		if (pos >= txtsize) bletch("relocation after end of segment");
/**		dread(&rel, sizeof rel, 1, rtext);*/
		get68(rtext, &rel.rinfo, sizeof(rel.rinfo));
		get68(rtext, &rel.rsymbol, sizeof(rel.rsymbol));
		get68(rtext, &rel.rpos, sizeof(rel.rpos));
		switch(rel.rinfo & RSEGMNT)
		{
		case REXT:
			offs = relext(&rel);
			break;
		case RTEXT:
			offs = torigin;
			break;
		case RDATA:
			offs = dorigin - filhdr.tsize;
			break;
		case RBSS:
			offs = borigin - (filhdr.tsize + filhdr.dsize);
			break;
		}
		if (rel.rinfo & RDISP)
			offs -= rel.rpos + txtstart;
		switch(rel.rinfo & RSIZE)
		{
		case RBYTE:
			size = 1; break;
		case RWORD:
			size = 2; break;
		case RLONG:
			size = 4; break;
		default:
			fatal(e12, filename);
		}
		if (rel.rpos > txtsize) fatal(e13, filename);
		else pos = relcmd(pos, rel.rpos, size, offs, outf);
		if (rflag && (rel.rinfo & RDISP)==0)	/* write out relocation commands */
		{
			rel.rpos +=  txtstart;
			rel.rsymbol = ((rel.rinfo & RSEGMNT) == REXT)?
				local[rel.rsymbol]->sindex: 0;
/**			fwrite(&rel, sizeof rel, 1, outrf);*/
			put68(outrf, &rel.rinfo, sizeof(rel.rinfo));
			put68(outrf, &rel.rsymbol, sizeof(rel.rsymbol));
			put68(outrf, &rel.rpos, sizeof(rel.rpos));
		}
	}
	dcopy(text, outf, txtsize - pos);
}
/* relext -	Find the offset of an REXT command and fix up the rel cmd.
 */

long relext(r)
register struct reloc *r;
{
	register symp sp;		/* pointer to symbol for EXT's */
	long offs;			/* return value */

	if ((r->rsymbol < 0 || r->rsymbol >= NSYMPR) ||
		((sp = local[r->rsymbol]) == NULL))
	{
		error(e19, filename);
		offs = 0;
	}
	else
	{
		offs = sp->s.svalue;
		if (sp->sname && strcmp(sp->sname, "bdevsw") == 0)
			fprintf(stderr, "DEBUG relext: bdevsw offs=0x%lx stype=0x%x rsymbol=%d rpos=0x%lx\n",
				offs, sp->s.stype, r->rsymbol, r->rpos);
		switch(sp->s.stype & 037)
		{
		case TEXT: (r->rinfo & ~RSEGMNT) | RTEXT; break;
		case DATA: (r->rinfo & ~RSEGMNT) | RDATA; break;
		case BSS: (r->rinfo & ~RSEGMNT) | RBSS; break;
		case UNDEF:
			if (rflag && (sp->s.stype & EXTERN))
			{
				r->rinfo = (r->rinfo & ~RSEGMNT) | REXT;
				offs = 0;
				break;
			}
/*		default: error(e20, filename, sp->sname);	*/
		}
	}
	return(offs);
}


/* relcmd -	Seek to <position> in file by copying from text to
		outf, then Add an offset to the next <size>
		bytes in the text and write out to outf.
 */

long relcmd(current, position, size, offs, outf)
long current;		/* current position in file */
long position;		/* where in file to perform relocation */
int size;		/* number of bytes to fix up */
long offs;		/* the number to fix up bye */
FILE *outf;		/* where to write to */
{
	register int i, c;
	long buf = 0;
	if (position < current)
	{
		error(e21, filename);
		return(current);
	}
	dcopy(text, outf, position - current);
	for (i = 0; i < size; i++)
	{
		if ((c = getc(text)) == EOF) fatal(e3, filename);
		buf = (buf << 8) | (c & 0377);
	}
	buf += offs;
	for (i = size - 1; i >= 0; i--)
	{
		c = (buf >> (i * 8)) & 0377;
		putc(c, outf);
	}
	return(position + size);
}
/* finishout 	Finish off the output file by writing out symbol table (nlist) */

finishout()
{
	register symp sp;

	{
		long symoff = HDRSIZE + tsize + dsize;
		if (rflag) symoff += rtsize + rdsize;
		fseek(tout, symoff, 0);
	}
	if (sflag == 0) for (sp = symtab; sp < &symtab[symindex]; sp++)
	{
		register int i;
		register char *cp;
		short n_type;
		unsigned short n_value;
		char n_name[8];

		/* Write 8-char name, NUL-padded */
		for (i = 0; i < 8; i++) n_name[i] = 0;
		cp = sp->sname;
		for (i = 0; i < 8 && *cp; i++) n_name[i] = *cp++;
		fwrite(n_name, 8, 1, tout);
		/* Write n_type (convert from internal encoding) */
		n_type = sp->s.stype >> 8;
		put68(tout, &n_type, 2);
		/* Write n_value */
		n_value = (short)sp->s.svalue;
		put68(tout, &n_value, 2);
		if (ferror(tout)) fatal(e14, ofilename);
	}
	fclose(tout);
	fclose(dout);
	if (rflag)
	{
		fclose(trout);
		fclose(drout);
	}
#ifdef z8000
	chmod(ofilename, 0777);
#endif
}
/* getfile -	Open an input file as text.  Returns the first word of
		file but leaves the file pointer at zero */
getfile(cp)
register char *cp;	/* name of the file to open */
{
	int cc;
	register int c;
	char buff[SARMAG];
	short magic;

	archdr.ar_name[0] = '\0';
	filename = cp;
	text = NULL;
	if (cp[0]=='-' && cp[1]=='l') {
		if(cp[2] == '\0') cp = "-la";
		strcpy(libname, libdir);
		strcat(libname, &cp[2]);
		filename = strcat(libname, ".a");
		if ((text = fopen(filename+4, "r")) != NULL) filename += 4;
	}
	if (text == NULL && ((text = fopen(filename, "r")) == NULL))
		fatal(e15, filename);
	if ((rtext = fopen(filename, "r")) == NULL) fatal(e15, filename);
	get68(text, &magic, 2);
	fseek(text, 0L, 0);	/* reset file pointer */
	if (magic == FMAGIC) return(FMAGIC);
	dread(buff, SARMAG, 1, text);
	fseek(text, 0L, 0);	/* reset file pointer */
	if (strncmp(buff, ARMAG, SARMAG) == 0)
	  return(ARCMAGIC);
	return(0);
}

/* lookup -	Returns the pointer into symtab of a symbol with the
		same name cursym.sname or NULL if none.
 */

symp lookup()
{
	return(*hash(cursym.sname));
}

/* hash -	Return a pointer to where in the hash table
		a symbol should go.
*/

symp *hash(s)
char *s;
{
	register int i = 0, cmpv;
	register int initial;
	register char *cp;

	i = 0;
	for (cp = s; *cp;) i = (i<<1) + *cp++;

	initial = i =(i&077777)%NSYM+2;
	while(hshtab[i])
	{
		cmpv = strcmp(hshtab[i]->sname, cursym.sname);
		if ((cmpv != 0) || ((cursym.s.stype & EXTERN) == 0))
		{
			if (++i  == NSYM+2) i = 0;
		}
		else break;
		if (i == initial) fatal(e16);
	}
	return(&hshtab[i]);
}

/* slookup -	Like lookup but with an arbitrary string argument */

symp slookup(s)
char *s;
{
	cursym.sname = s;
	cursym.snlength = strlen(s);
	cursym.s.stype = EXTERN+UNDEF;
	cursym.s.svalue = 0;
	return(lookup());
}

/* enter -	Make sure that cursym is installed in symbol table.
		Called with a pointer to a symbol with the same name
		or NULL if lookup failed.  Returns 1 if the symbol
		was new, or 0 if it was already present.
*/

enter(sp)
register symp sp;
{
	symp *hp;		/* pointer to place in hash table */
	if (sp == NULL)
	{
		hp = hash(cursym.sname);
		if (symindex>=NSYM) fatal(e17);
		lastsym = sp = &symtab[symindex];
		if (*hp) bletch("hash table conflict");
		*hp = sp;
		if((sp->sname = (char *)calloc(1, cursym.snlength+1)) == NULL)
		  fatal(e17);
		strcpy(sp->sname, cursym.sname);
		if (*(sp->sname) == 0) bletch("null symbol entered");
		sp->snlength = cursym.snlength;
		sp->s.stype = cursym.s.stype;
		sp->s.svalue = cursym.s.svalue;
		sp->shash = hp;
		sp->sindex = symindex++;
		return(1);
	}
	else
	{
		lastsym = sp;
		return(0);
	}
}

/* symrel -	Perform partial relocation of symbol.  Each symbol is
		relocated twice.  The first time, it is adjusted to
		is relative position within its segment.  The second
		time, it is adjusted by the start of the final segment
		to which that symbol refers.  This routine only
		performs the first relocation.
 */

symreloc()
{
	switch (cursym.s.stype) {

	case TEXT:
	case EXTERN+TEXT:
		cursym.s.svalue += ctrel;
		return;

	case DATA:
	case EXTERN+DATA:
		cursym.s.svalue += cdrel;
		return;

	case BSS:
	case EXTERN+BSS:
		cursym.s.svalue += cbrel;
		return;

	case EXTERN+UNDEF:
		return;
	}
	if (cursym.s.stype&EXTERN)
		cursym.s.stype = EXTERN+ABS;
}
/* readhdr -	Read in an a.out header (16 bytes), adjusting relocation offsets */
readhdr(pos)
long pos;
{
	register long st, sd;
	unsigned short tmp;
	fseek(text, pos, 0);
	get68(text, &tmp, 2); filhdr.fmagic = tmp;
	get68(text, &tmp, 2); filhdr.tsize = tmp;
	get68(text, &tmp, 2); filhdr.dsize = tmp;
	get68(text, &tmp, 2); filhdr.bsize = tmp;
	get68(text, &tmp, 2); filhdr.ssize = tmp;
	get68(text, &tmp, 2); filhdr.entry = tmp;
	get68(text, &tmp, 2); filhdr.trsize = tmp;
	get68(text, &tmp, 2); filhdr.drsize = tmp;
	if (filhdr.fmagic != FMAGIC)
		error(e5, filename);
	st = (filhdr.tsize+1) & ~1;
	filhdr.tsize = st;
	cdrel = -st;
	sd = (filhdr.dsize+1) & ~1;
	cbrel = - (st + sd);
	filhdr.bsize = (filhdr.bsize+1) & ~1;
}
/* getsym -	Read a 12-byte nlist entry from text, leaving data in cursym.
		Returns NLIST_DISKSIZE (12). */

long getsym()
{
	register int i;
	short n_type;
	unsigned short n_value;
	char n_name[8];

	/* Read 8-byte name */
	if (fread(n_name, 8, 1, text) != 1) fatal(e3, filename);
	/* Read 2-byte n_type, convert to internal encoding */
	get68(text, &n_type, 2);
	cursym.s.stype = n_type << 8;
	/* Read 2-byte n_value */
	get68(text, &n_value, 2);
	cursym.s.svalue = n_value;
	/* Copy name to csymbuf */
	for (i = 0; i < 8; i++) csymbuf[i] = n_name[i];
	csymbuf[8] = '\0';
	/* Find actual length */
	for (i = 0; i < 8 && n_name[i]; i++);
	if (i == 0) fatal(e18, filename);
	cursym.snlength = i;
	cursym.sname = csymbuf;
	return(NLIST_DISKSIZE);
}

/* error -	Print out error messages and give up if they are severe. */

/*VARARGS1*/
error(fmt, a1, a2, a3, a4, a5)
char *fmt;
{
	nerrors++;
	printf("ldz8: ");
	printf(fmt, a1, a2, a3, a4, a5);
	printf("\n");
}


/* fatal -	Print error message and exit */

/*VARARGS1*/
fatal(fmt, a1, a2, a3, a4, a5)
char *fmt;
{
	error(fmt, a1, a2, a3, a4, a5);
	exit(1);
}


/* bletch -	Print out a message and abort.  Used for internal errors. */

/*VARARGS1*/
bletch(fmt, a1, a2, a3, a4, a5)
char *fmt;
{
	error(fmt, a1, a2, a3, a4, a5);
	abort();
}
/* dread -	Like fread but checks for errors and eof */

dread(pos, size, count, file)
char *pos;
int size;
int count;
FILE *file;
{
	if (fread(pos, size, count, file) == size * count) return;
	if (feof(file)) fatal(e3, filename);
	if (ferror(file)) fatal(e4, filename);
}


/* dcopy -	Copy input to output, checking for errors */

dcopy(in, out, count)
FILE *in, *out;
long count;
{
	register int c;
	long n = count;
	if (n < 0) bletch("bad count to dcopy");
	while(n--)
	{
		if ((c = getc(in)) == EOF)
		{
			if (feof(in)) fatal(e3, filename);
			if (ferror(out)) fatal(e4, filename);
		}
		putc(c, out);
	}
}


get68(file, p, c)
register FILE	*file;
register char	*p;
register c;
{
	if(c == 2) {
		*(short *)p = getc(file);
		*(short *)p = *(short *)p << 8 | getc(file);
	}
	else {
		*(long *)p = getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
	}

	if (feof(file)) fatal(e3, filename);
	if (ferror(file)) fatal(e4, filename);
}


put68(file, p, c)
register FILE	*file;
register char	*p;
{
	if(c == 2) {
		putc(*(short *)p >> 8, file);
		putc(*(short *)p, file);
	}
	else {
		putc(*(long *)p >> 24, file);
		putc(*(long *)p >> 16, file);
		putc(*(long *)p >> 8, file);
		putc(*(long *)p, file);
	}
}
