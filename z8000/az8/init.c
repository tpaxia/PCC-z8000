#include "mical.h"
#include "inst.h"

char Title[STR_MAX];
char O_outfile = 0;		/* 1 if .rel file name is specified by user */
int Cflag = 0;
int Pass = 0;			/* which pass we're on */
char Rel_name[STR_MAX];		/* Name of .rel file */
FILE *Rel_file;			/* and ptr to it */
struct sym_bkt *Dot_bkt ;	/* Ptr to location counter's symbol bucket */
long tsize = 0;			/* sizes of three main csects */
long dsize = 0;
long bsize = 0;
struct ins_bkt *ins_hash_tab[HASH_MAX];

/* List of Z8000 op codes */
struct ins_init { char *opstr; short opnum; } op_codes[] = {
	/* load and move */
	"ld",	i_ld,
	"ldb",	i_ldb,
	"ldl",	i_ldl,
	"lda",	i_lda,
	"ldk",	i_ldk,
	"ldm",	i_ldm,
	"ldr",	i_ldr,
	"ldrb",	i_ldrb,
	"ldrl",	i_ldrl,
	"ex",	i_ex,
	"exb",	i_exb,
	/* arithmetic */
	"add",	i_add,
	"addb",	i_addb,
	"addl",	i_addl,
	"adc",	i_adc,
	"adcb",	i_adcb,
	"sub",	i_sub,
	"subb",	i_subb,
	"subl",	i_subl,
	"sbc",	i_sbc,
	"sbcb",	i_sbcb,
	"inc",	i_inc,
	"incb",	i_incb,
	"dec",	i_dec,
	"decb",	i_decb,
	"neg",	i_neg,
	"negb",	i_negb,
	"da",	i_da,
	"dab",	i_dab,
	/* compare */
	"cp",	i_cp,
	"cpb",	i_cpb,
	"cpl",	i_cpl,
	/* logical */
	"and",	i_and,
	"andb",	i_andb,
	"or",	i_or,
	"orb",	i_orb,
	"xor",	i_xor,
	"xorb",	i_xorb,
	"com",	i_com,
	"comb",	i_comb,
	"test",	i_test,
	"testb",	i_testb,
	"testl",	i_testl,
	/* bit manipulation */
	"bit",	i_bit,
	"bitb",	i_bitb,
	"set",	i_set,
	"setb",	i_setb,
	"res",	i_res,
	"resb",	i_resb,
	"tset",	i_tset,
	"tsetb",	i_tsetb,
	/* shift and rotate */
	"sla",	i_sla,
	"slab",	i_slab,
	"slal",	i_slal,
	"sra",	i_sra,
	"srab",	i_srab,
	"sral",	i_sral,
	"sll",	i_sll,
	"sllb",	i_sllb,
	"slll",	i_slll,
	"srl",	i_srl,
	"srlb",	i_srlb,
	"srll",	i_srll,
	"sda",	i_sda,
	"sdab",	i_sdab,
	"sdal",	i_sdal,
	"sdl",	i_sdl,
	"sdlb",	i_sdlb,
	"sdll",	i_sdll,
	"rl",	i_rl,
	"rlb",	i_rlb,
	"rlc",	i_rlc,
	"rlcb",	i_rlcb,
	"rr",	i_rr,
	"rrb",	i_rrb,
	"rrc",	i_rrc,
	"rrcb",	i_rrcb,
	/* multiply and divide */
	"mult",	i_mult,
	"multl",	i_multl,
	"div",	i_div,
	"divl",	i_divl,
	/* sign extend */
	"exts",	i_exts,
	"extsb",	i_extsb,
	"extsl",	i_extsl,
	/* clear */
	"clr",	i_clr,
	"clrb",	i_clrb,
	/* program control */
	"call",	i_call,
	"calr",	i_calr,
	"ret",	i_ret,
	"jp",	i_jp,
	"jr",	i_jr,
	"djnz",	i_djnz,
	"dbjnz",	i_dbjnz,
	/* conditional jr */
	"jr eq",	i_jreq,
	"jr ne",	i_jrne,
	"jr lt",	i_jrlt,
	"jr le",	i_jrle,
	"jr gt",	i_jrgt,
	"jr ge",	i_jrge,
	"jr ult",	i_jrult,
	"jr ule",	i_jrule,
	"jr ugt",	i_jrugt,
	"jr uge",	i_jruge,
	"jr mi",	i_jrmi,
	"jr pl",	i_jrpl,
	"jr ov",	i_jrov,
	"jr nov",	i_jrnov,
	"jr c",	i_jrc,
	"jr nc",	i_jrnc,
	/* conditional jp */
	"jp eq",	i_jpeq,
	"jp ne",	i_jpne,
	"jp lt",	i_jplt,
	"jp le",	i_jple,
	"jp gt",	i_jpgt,
	"jp ge",	i_jpge,
	"jp ult",	i_jpult,
	"jp ule",	i_jpule,
	"jp ugt",	i_jpugt,
	"jp uge",	i_jpuge,
	"jp mi",	i_jpmi,
	"jp pl",	i_jppl,
	"jp ov",	i_jpov,
	"jp nov",	i_jpnov,
	"jp c",	i_jpc,
	"jp nc",	i_jpnc,
	/* stack */
	"push",	i_push,
	"pushl",	i_pushl,
	"pop",	i_pop,
	"popl",	i_popl,
	/* block transfer and search */
	"ldir",	i_ldir,
	"ldirb",	i_ldirb,
	"lddr",	i_lddr,
	"lddrb",	i_lddrb,
	"ldi",	i_ldi,
	"ldib",	i_ldib,
	"ldd",	i_ldd,
	"lddb",	i_lddb,
	"cpir",	i_cpir,
	"cpirb",	i_cpirb,
	"cpdr",	i_cpdr,
	"cpdrb",	i_cpdrb,
	"cpi",	i_cpi,
	"cpib",	i_cpib,
	"cpd",	i_cpd,
	"cpdb",	i_cpdb,
	/* I/O */
	"in",	i_in,
	"inb",	i_inb,
	"out",	i_out,
	"outb",	i_outb,
	"ind",	i_ind,
	"indb",	i_indb,
	"outd",	i_outd,
	"outdb",	i_outdb,
	"indr",	i_indr,
	"indrb",	i_indrb,
	"otdr",	i_otdr,
	"otdrb",	i_otdrb,
	"ini",	i_ini,
	"inib",	i_inib,
	"outi",	i_outi,
	"outib",	i_outib,
	"inir",	i_inir,
	"inirb",	i_inirb,
	"otir",	i_otir,
	"otirb",	i_otirb,
	/* special */
	"nop",	i_nop,
	"halt",	i_halt,
	"di",	i_di,
	"ei",	i_ei,
	"sc",	i_sc,
	"iret",	i_iret,
	"mbit",	i_mbit,
	"mreq",	i_mreq,
	"mres",	i_mres,
	"mset",	i_mset,
	"ldctl",	i_ldctl,
	"ldctlb", i_ldctlb,
	/* pseudo ops */
	".long", i_long,
	".word", i_word,
	".byte", i_byte,
	".text", i_text,
	".data", i_data,
	".bss", i_bss,
	".globl", i_globl,
	".comm", i_comm,
	".even", i_even,
	".asciz", i_asciz,
	".ascii", i_ascii,
	".zerol", i_zerol,
	".zerow", i_zerow,
	0 };

char *Source_name = NULL;
char File_name[STR_MAX];

Init(argc,argv)
char *argv[];
{	register int i,j;
	char *strncpy();
	char *cp1, *cp2, *end, *rindex();

	argv++;
	while (--argc > 0) {
	  if (argv[0][0] == '-') switch (argv[0][1]) {
	    case 'o':	O_outfile++;
			Concat(Rel_name,argv[1],"");
			argv++; argc--;
			break;

	    case 'c':	Cflag++;
			break;

	    default:	fprintf(stderr,"Unknown option '%c' ignored.\n",argv[0][1]);
	  } else if (Source_name != NULL) {
	    fprintf(stderr,"Too many file names given\n");
	  } else {
	    Source_name = argv[0];
	    Concat(File_name, argv[0], ".az8");
	    if (freopen(File_name,"r",stdin) == NULL) {
	      if ((end = rindex(Source_name, '.')) == 0 ||
			strcmp(end, ".az8") != 0) {
	        fprintf(stderr,"Can't open source file: %s\n",File_name);
	        exit(1);
	      }
	      strncpy(File_name, argv[0], STR_MAX);
	      if (freopen(File_name,"r",stdin) == NULL) {
	        fprintf(stderr,"Can't open source file: %s\n",File_name);
	        exit(1);
	      }
	    }
	  }
	  argv++;
	}


/* Check to see if we can open output file */
	if(!O_outfile)
	{
		if ((end = rindex(Source_name, '.')) == 0 ||
			strcmp(end, ".az8") != 0)
			Concat(Rel_name,Source_name,".b");
		else	/* copy basename without .az8 to Rel_name */
		{
			for (cp1 = Source_name, cp2 = Rel_name; cp1 < end;)
				*cp2++ = *cp1++;
			strcpy(cp2, ".b");
		}
	}
	if ((Rel_file = fopen(Rel_name,"w")) == NULL)
	{	printf("Can't create output file: %s\n",Rel_name);
		exit(1);
	}
	fclose(Rel_file);	/* Rel_Header will open properly */

/* Initialize symbols */
	Sym_Init();
	Dot_bkt = Lookup(".");		/* make bucket for location counter */
	Dot_bkt->csect_s = Cur_csect;
	Dot_bkt->attr_s = S_DEC | S_DEF | S_LABEL;
	init_regs();			/* define register names */
	d_ins();			/* set up opcode hash table */
	Perm();
	Start_Pass();
}

d_ins()
{	register struct ins_init *p;
	register struct ins_bkt *insp;
	register int save;

	for (p = op_codes; p->opstr != 0; p++) {
		insp = (struct ins_bkt *)calloc(1,sizeof(struct ins_bkt));
		insp->text_i = p->opstr;
		insp->code_i = p->opnum;
		insp->next_i = ins_hash_tab[save = Hash(insp->text_i)];
		ins_hash_tab[save] = insp;
	}
}

/* Z8000 register definitions.
 * Word registers: .r0-.r15 (values 0-15)
 * Byte registers: .rh0-.rh7 (values 16-23), .rl0-.rl7 (values 24-31)
 * Long registers: .rr0,.rr2,...,.rr14 (values 32+regnum)
 * Quad registers: .rq0,.rq4,.rq8,.rq12 (values 48+regnum)
 * Special: .sp = r15
 */
struct def { char *rname; int rnum; }
defregs[] = {
  "r0", 0, "r1", 1, "r2", 2, "r3", 3,
  "r4", 4, "r5", 5, "r6", 6, "r7", 7,
  "r8", 8, "r9", 9, "r10", 10, "r11", 11,
  "r12", 12, "r13", 13, "r14", 14, "r15", 15,
  "sp", 15,
  "rh0", 16, "rh1", 17, "rh2", 18, "rh3", 19,
  "rh4", 20, "rh5", 21, "rh6", 22, "rh7", 23,
  "rl0", 24, "rl1", 25, "rl2", 26, "rl3", 27,
  "rl4", 28, "rl5", 29, "rl6", 30, "rl7", 31,
  "rr0", 32, "rr2", 34, "rr4", 36, "rr6", 38,
  "rr8", 40, "rr10", 42, "rr12", 44, "rr14", 46,
  "rq0", 48, "rq4", 52, "rq8", 56, "rq12", 60,
  "fcw", 64, "refresh", 65, "psapseg", 66, "psapoff", 67,
  "nspseg", 68, "nspoff", 69,
  "flags", 70,
  0, 0
},
cdefregs[] = {
  ".r0", 0, ".r1", 1, ".r2", 2, ".r3", 3,
  ".r4", 4, ".r5", 5, ".r6", 6, ".r7", 7,
  ".r8", 8, ".r9", 9, ".r10", 10, ".r11", 11,
  ".r12", 12, ".r13", 13, ".r14", 14, ".r15", 15,
  ".sp", 15,
  ".rh0", 16, ".rh1", 17, ".rh2", 18, ".rh3", 19,
  ".rh4", 20, ".rh5", 21, ".rh6", 22, ".rh7", 23,
  ".rl0", 24, ".rl1", 25, ".rl2", 26, ".rl3", 27,
  ".rl4", 28, ".rl5", 29, ".rl6", 30, ".rl7", 31,
  ".rr0", 32, ".rr2", 34, ".rr4", 36, ".rr6", 38,
  ".rr8", 40, ".rr10", 42, ".rr12", 44, ".rr14", 46,
  ".rq0", 48, ".rq4", 52, ".rq8", 56, ".rq12", 60,
  0, 0
};

init_regs()
  {	register struct sym_bkt *sbp;
	register struct def *p;
	struct sym_bkt *Lookup();

	/* always load both standard (r0) and C-style (.r0) register names */
	p = defregs;
	while (p->rname) {
	  sbp = Lookup(p->rname);
	  sbp->value_s = p->rnum;
	  sbp->attr_s |= S_REG | S_DEC | S_DEF;
	  p++;
	}
	p = cdefregs;
	while (p->rname) {
	  sbp = Lookup(p->rname);	/* Make a sym_bkt for it */
	  sbp->value_s = p->rnum;	/* Load the sym_bkt */
	  sbp->csect_s = 0;
	  sbp->attr_s = S_DEC | S_DEF | S_REG;
	  p++;
	}
}

Concat(s1,s2,s3)
  register char *s1,*s2,*s3;
  {	while (*s1++ = *s2++);
	s1--;
	while (*s1++ = *s3++);
}


/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
*/

#define NULL 0

char *
rindex(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}


/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
strncpy(s1, s2, n)
register char *s1, *s2;
{
	register i;
	register char *os1;

	os1 = s1;
	for (i = 0; i < n; i++)
		if ((*s1++ = *s2++) == '\0') {
			while (++i < n)
				*s1++ = '\0';
			return(os1);
		}
	return(os1);
}
