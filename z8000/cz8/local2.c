# include "mfile2"

NODE *block(), *makety();
/* a lot of the machine dependent parts of the second pass */

# define BITMASK(n) ((1L<<n)-1)

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "!	line %d, file %s\n", l, fn );
	}

int usedregs;	/* flag word for registers used in current routine */
int maxtoff = 0;

cntbits(i)
  register int i;
  {	register int j,ans;

	for (ans=0, j=0; i!=0 && j<16; j++) { if (i&1) ans++; i>>= 1; }
	return(ans);
}

eobl2(){
	extern int retlab;
	OFFSZ spoff;	/* offset from stack pointer */
	int savemask;

	spoff = maxoff;
	spoff /= SZCHAR;
	SETOFF(spoff,2);

	/* callee-saved regs are always saved in prologue at fixed offsets */
	/* AUTOINIT reserves 16 bytes: R4-R7 at -2..-8, R10-R12,R14 at -10..-16 */
	savemask = usedregs & 0x5CF0;  /* bits for R4-R7, R10-R12, R14 */

	/* epilogue */
	printf( ".L%d:\n", retlab );

	/* Shared return restores the fixed slots without disturbing R0-R3. */
	printf( "\tjp\tcret\n" );
	printf( ".F%d = %ld\n", ftnno, spoff );
	printf( ".S%d = %d\n", ftnno, savemask );
	printf( "! M%d = %d\n", ftnno, maxtoff );
	maxtoff = 0;
	if( fltused ) {
		fltused = 0;
		printf( "	.globl	fltused\n" );
		}
	}

struct hoptab { int opmask; char * opstring; } ioptab[]= {

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG OR,	"or",
	ASG AND, "and",
	ASG ER,	"xor",
	ASG LS,	"sla",
	ASG RS,	"sra",

	-1, ""    };

hopcode( f, o ){
	/* output the appropriate string from the above table */

	register struct hoptab *q;

	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			printf( "%s", q->opstring );
			return;
			}
		}
	cerror( "no hoptab for %s", opst[o] );
	}

char *
rnames[]= {  /* keyed to register number tokens */

	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "sp"
	};

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG,	/* R0, R1: scratch data */
	SAREG|STAREG, SAREG|STAREG,	/* R2, R3: scratch data */
	SAREG|STAREG, SAREG|STAREG,	/* R4, R5: reg var data */
	SAREG|STAREG, SAREG|STAREG,	/* R6, R7: reg var data */

	SBREG|STBREG, SBREG|STBREG,	/* R8, R9: scratch addr */
	SBREG|STBREG, SBREG|STBREG,	/* R10, R11: reg var addr */
	SBREG|STBREG, SBREG,		/* R12: reg var addr, R13 (FP): not allocatable */
	SBREG,	      SBREG,		/* R14 (reserved), R15 (SP): not allocatable */
	};

NODE *brnode;
int brcase;

int toff = 0; /* number of stack locations used for args */

zzzcode( p, c ) NODE *p; {
	register m,temp;
	switch( c ){

	case 'C':
		/* function call */
		switch (p->in.left->in.op) {
		  case ICON:	printf("\tcall\t");
				acon(p->in.left);
				return;

		  case NAME:	/* indirect call through function pointer variable */
				printf("\tld\tr8,");
				adrput(p->in.left);
				printf("\n\tcall\t@r8");
				return;

		  case REG:	printf("\tcall\t@");
				adrput(p->in.left);
				return;

		  case OREG:	printf("\tld\tr8,");
				adrput(p->in.left);
				printf("\n\tcall\t@r8");
				return;

		  default:	cerror("bad subroutine name");
		}

	case 'B':
		m = p->in.type;
		/* type suffix: b for byte, nothing for word, l for long */
		if( m == CHAR || m == UCHAR ) printf( "b" );
		return;

	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	clr	%s\n", rnames[temp = getlr( p, '1' )->tn.rval] );
		usedregs |= 1<<temp;
		deflab( m );
		return;

	case 'I':
		cbgen( p->in.op, p->bn.label, c );
		return;

		/* stack management macros */
	case '-':
		/* push word onto stack */
		toff += 2;
		if (toff > maxtoff) maxtoff = toff;
		return;

	case 'P':
		toff += 2;
		if (toff > maxtoff) maxtoff = toff;
		return;

	case '0':
		toff = 0; return;

	case '~':
		/* complimented CR */
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = ~p->in.right->tn.lval;
		return;

	case 'M':
		/* negated CR */
		p->in.right->tn.lval = -p->in.right->tn.lval;
	case 'O':
		conput( getlr( p, 'R' ) );
		p->in.right->tn.lval = -p->in.right->tn.lval;
		return;

	case 'T':
		/* Truncate for type conversions:
		   LONG -> INT/SHORT/CHAR
		   increment offset to second word of long */
		m = p->in.type;
		p = p->in.left;
		switch( p->in.op ){
		case NAME:
		case OREG:
			if( m == CHAR || m == UCHAR )
				p->tn.lval += 1;  /* byte 1 of word */
			/* for short/int from long, add 2 to get low word */
			else if( p->in.type == LONG || p->in.type == ULONG )
				p->tn.lval += 2;
			return;
		case REG:
			return;
		default:
			cerror( "Illegal ZT type conversion" );
			return;

			}

	case 'L':
		/* set up long comparison for cbgen */
		brcase = 'C';
		brnode = p;
		return;

	case 'D':
		/* print register pair name for escape register 1 */
		{
			register int r;
			r = getlr( p, '1' )->tn.rval;
			printf( "rr%d", r & ~1 );
		}
		return;

	case 'V': /* unsigned word division, including divisors >= 32768 */
		{
			int large = getlab(), less = getlab(), done = getlab();
			/* Preserve the scratch address register; capture RHS before
			 * moving the dividend into the fixed division register. */
			expand(p, FOREFF, "\tpush\t@sp,r9\n\tld\tr9,AR\n\tld\tr1,AL\n");
			printf("\ttest\tr9\n\tjr mi,.L%d\n", large);
			printf("\tclr\tr0\n\tdiv\trr0,r9\n\tjr\t.L%d\n", done);
			deflab(large);
			printf("\tcp\tr1,r9\n\tjr ult,.L%d\n", less);
			printf("\tsub\tr1,r9\n\tld\tr0,r1\n\tld\tr1,#1\n\tjr\t.L%d\n", done);
			deflab(less);
			printf("\tld\tr0,r1\n\tclr\tr1\n");
			deflab(done);
			if(p->in.op==MOD || p->in.op==ASG MOD)
				printf("\tld\tr1,r0\n");
			printf("\tpop\tr9,@sp\n");
			if(asgop(p->in.op)) expand(p, FOREFF, "\tld\tAL,r1\n");
		}
		return;

	case 'X': /* copy a double between addressable operands */
	case 'E': /* load a double into the result register quad */
	case 'G': /* push a double, low word first */
	case 'K': /* spill a double to a frame temporary */
		{
			NODE *src = getlr(p,'R');
			NODE *dst = c=='X' ? getlr(p,'L') : getlr(p,'1');
			int k, words = src->in.type==FLOAT ? 2 : 4;
			for(k=0;k<words;k++) {
				if(c=='G') {
					printf("\tpush\t@sp,"); fpword(src,words-1-k); printf("\n");
				} else if(src->in.op!=REG && dst->in.op!=REG) {
					printf("\tld\t%s,",rnames[getlr(p,'1')->tn.rval]);
					fpword(src,k); printf("\n\tld\t"); fpword(dst,k);
					printf(",%s\n",rnames[getlr(p,'1')->tn.rval]);
				} else {
					printf("\tld\t"); fpword(dst,k); printf(","); fpword(src,k); printf("\n");
				}
			}
			if(c=='G') { toff+=2*words; if(toff>maxtoff) maxtoff=toff; }
		}
		return;

	case 'Y': /* floating truth: ignore sign, test all magnitude bits */
		{
			NODE *src=getlr(p,'R'), *dst=getlr(p,'1');
			int k,words=src->in.type==FLOAT?2:4;
			printf("\tld\t%s,",rnames[dst->tn.rval]); fpword(src,0);
			printf("\n\tand\t%s,#32767\n",rnames[dst->tn.rval]);
			for(k=1;k<words;k++) {
				printf("\tor\t%s,",rnames[dst->tn.rval]); fpword(src,k); printf("\n");
			}
		}
		return;

	case 'J': /* high and low words of a long static initializer */
		if( p->in.left->in.name[0] ) {
			/* an address: it is 16 bits wide, so the high word is 0 */
			printf("\t.word\t0\n\t.word\t");
			acon(p->in.left);
			printf("\n");
			return;
		}
		printf("\t.word\t%ld\n\t.word\t%ld\n",
		    (p->in.left->tn.lval >> 16) & 65535L,
		    p->in.left->tn.lval & 65535L);
		return;

	case 'Q':
		/* print register pair name for left operand (for long shifts) */
		{
			register int r;
			r = getlr( p, 'L' )->tn.rval;
			printf( "rr%d", r & ~1 );
		}
		return;

	case 'U':
		cerror( "Illegal ZU" );
		/* NO RETURN */

	case 'W':	/* structure size */
		if( p->in.op == STASG )
			printf( "%d", p->stn.stsize);
		else	cerror( "Not a structure" );
		return;

	case 'S':  /* structure assignment */
		{
			register NODE *l, *r;
			register size;

			if( p->in.op == STASG ){
				l = p->in.left;
				r = p->in.right;
				}
			else if( p->in.op == STARG ){  /* store an arg onto the stack */
				r = p->in.left;
				}
			else cerror( "STASG bad" );

			if( r->in.op == ICON ) r->in.op = NAME;
			else if( r->in.op == REG ) r->in.op = OREG;
			else if( r->in.op != OREG ) cerror( "STASG-r" );

			size = p->stn.stsize;

			{
				/* Copy exact bytes, including large and odd-sized aggregates. */
				r->tn.lval += size;
				if( p->in.op == STASG ) l->tn.lval += size;

				while( size ){
					int i;
					i = (size > 1) ? 2 : 1;
					r->tn.lval -= i;
					if( i == 2 ){
						expand( r, FOREFF, "\tld\tr0,AR\n" );
					} else {
						expand( r, FOREFF, "\tldb\trl0,AR\n" );
					}
					if( p->in.op == STASG ){
						l->tn.lval -= i;
						if( i == 2 ){
							expand( l, FOREFF, "\tld\tAR,r0\n" );
						} else {
							expand( l, FOREFF, "\tldb\tAR,rl0\n" );
						}
					} else {
						if( i == 2 )
							printf( "\tpush\t@sp,r0\n" );
						else {
							/* push byte as word */
							printf( "\textsb\tr0\n" );
							printf( "\tpush\t@sp,r0\n" );
						}
					}
					size -= i;
				}
			}

			if( r->in.op == NAME ) r->in.op = ICON;
			else if( r->in.op == OREG ) r->in.op = REG;

			}
		break;

	case 'H':
		/* After mult/div into rr0, update left child to reflect
		   that the result is now in R1 (not whatever register
		   the left operand was originally in).  Without this,
		   RLEFT/reclaim tracks the wrong register.
		   Must also fix busy[] accounting: recl2 in walkf will
		   rfree whatever register the left child shows. */
		{
			NODE *l = getlr( p, 'L' );
			int oldreg = l->tn.rval;
			if( oldreg != R1 ){
				rbusy( R1, p->in.type );
				if( istreg( oldreg ) )
					rfree( oldreg, p->in.type );
			}
			l->in.op = REG;
			l->tn.rval = R1;
			l->tn.lval = 0;
			l->in.name[0] = '\0';
		}
		return;

	default:
		cerror( "illegal zzzcode" );
		}
	}

rmove( rt, rs, t ) TWORD t; {
	int k, size = szty(t), i;
	for(k=0;k<size;k++) {
		i = rt>rs ? size-1-k : k;
		printf("\tld\t%s,%s\n",rnames[rt+i],rnames[rs+i]);
		usedregs |= (1<<(rs+i)) | (1<<(rt+i));
	}
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|SNAME|STARNM|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTAREG,	SOREG|SNAME|INAREG,
	0,	0 };

setregs(){ /* set up temporary registers */
	register i;
	register int naregs = (maxtreg>>8)&0377;

	/* use any unused variable registers as scratch registers */
	maxtreg &= 0377;
	fregs = maxtreg>=MINRVAR ? maxtreg + 1 : MINRVAR;
	if( xdebug ){
		/* -x changes number of free regs to 2, -xx to 3, etc. */
		if( (xdebug+1) < fregs ) fregs = xdebug+1;
		}

	for( i=MINRVAR; i<=MAXRVAR; i++ )
		rstatus[i] = i<fregs ? SAREG|STAREG : SAREG;
	for( i=MINRVAR; i<=MAXRVAR; i++ )
		rstatus[i+8] = i<naregs ? SBREG|STBREG : SBREG;
	rstatus[13] = SBREG;	/* R13 (FP): never allocatable */
	rstatus[15] = SBREG;	/* R15 (SP): never allocatable */
	}

szty(t) TWORD t; { /* size, in words, needed to hold thing of type t */
	/* really is the number of registers to hold type t */
	/* on Z8000: LONG and FLOAT need 2 regs (register pair) */
	/* DOUBLE uses a quad; arithmetic is implemented by library calls. */
	switch(t) {
	case DOUBLE:
		return(4);
	case LONG:
	case ULONG:
	case FLOAT:
		return(2);
	default:
		return(1);
	}
}

rewfld( p ) NODE *p; {
	return(1);
	}

callreg(p) NODE *p; {
	return( R0 );
	}

shltype( o, p ) NODE *p; {
	if( o == NAME|| o==REG || o == ICON || o == OREG ) return( 1 );
	return( o==UNARY MUL && p && shumul(p->in.left) );
	}

flshape( p ) register NODE *p; {
	register o = p->in.op;
	if( o==NAME || o==REG || o==ICON || o==OREG ) return( 1 );
	return( o==UNARY MUL && shumul(p->in.left)==STARNM );
	}

shtemp( p ) register NODE *p; {
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == REG || p->in.op == OREG ) return( !istreg( p->tn.rval ) );
	return( p->in.op == NAME || p->in.op == ICON );
	}

spsz( t, v ) TWORD t; CONSZ v; {

	/* is v the size to increment something of type t */

	if( !ISPTR(t) ) return( 0 );
	t = DECREF(t);

	if( ISPTR(t) ) return( v == 2 );  /* pointer to pointer: 2 bytes */

	switch( t ){

	case UCHAR:
	case CHAR:
		return( v == 1 );

	case SHORT:
	case USHORT:
	case INT:
	case UNSIGNED:
		return( v == 2 );

	case LONG:
	case ULONG:
	case FLOAT:
		return( v == 4 );

	case DOUBLE:
		return( v == 8 );
		}

	return( 0 );
	}

indexreg( p ) register NODE *p; {
	/* R1-R15 can be index registers, R0 cannot */
	if ( p->in.op==REG && p->tn.rval>=R1 && p->tn.rval<=SP) return(1);
	return(0);
}

shumul( p ) register NODE *p; {
	register o;

	o = p->in.op;
	if( indexreg(p) ) return( STARNM );

	/* Z8000 has no auto-increment addressing, so no STARREG */
	return( 0 );
	}

adrcon( val ) CONSZ val; {
	printf( CONFMT, val );
	}

conput( p ) register NODE *p; {
	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		printf( "%s", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) NODE *p; {
	cerror( "insput" );
	}

upput( p ) NODE *p; {
	/* output the address of the second word in the
	   pair pointed to by p (for LONGs)*/
	CONSZ save;

	if( p->in.op == FLD ){
		p = p->in.left;
		}

	save = p->tn.lval;
	switch( p->in.op ){

	case NAME:
		p->tn.lval += SZINT/SZCHAR;
		acon( p );
		break;

	case ICON:
		/* addressable value of the constant - low word */
		p->tn.lval &= BITMASK(SZINT);
		printf( "#" );
		acon( p );
		break;

	case REG:
		printf( "%s", rnames[p->tn.rval+1] );
		usedregs |= 1<<(p->tn.rval + 1);
		break;

	case OREG:
		p->tn.lval += SZINT/SZCHAR;
		if( p->tn.rval == FP ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			}
		if( p->tn.lval != 0 || p->in.name[0] != '\0' ) {
			acon( p );
			printf( "(%s)", rnames[p->tn.rval] );
		} else {
			printf( "@%s", rnames[p->tn.rval] );
		}
		usedregs |= 1<<p->tn.rval;
		break;

	default:
		cerror( "illegal upper address" );
		break;

		}
	p->tn.lval = save;

	}

adrput( p ) register NODE *p; {
	/* output an address, with offsets, from p */

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		return;

	case ICON:
		/* addressable value of the constant */
		if( szty( p->in.type ) == 2 ) {
			/* print the high order value for long */
			CONSZ save;
			if( p->in.name[0] ) {
				/* an address is 16 bits: its high word is 0 */
				printf( "#0" );
				return;
				}
			save = p->tn.lval;
			p->tn.lval = ( p->tn.lval >> SZINT ) & BITMASK(SZINT);
			printf( "#" );
			acon( p );
			p->tn.lval = save;
			return;
			}
		printf( "#" );
		acon( p );
		return;

	case REG:
		printf( "%s", rnames[p->tn.rval] );
		usedregs |= 1<<p->tn.rval;
		return;

	case OREG:
		/* Z8000 addressing: offset(Rn) or @Rn */
		usedregs |= 1<<p->tn.rval;
		if( p->tn.lval != 0 || p->in.name[0] != '\0' ) {
			acon( p );
			printf( "(%s)", rnames[p->tn.rval] );
		} else {
			printf( "@%s", rnames[p->tn.rval] );
		}
		return;

	case UNARY MUL:
		/* STARNM found (no STARREG on Z8000) */
		if( tshape(p, STARNM) ) {
			printf( "@" );
			adrput( p->in.left);
			}
		else {
			cerror( "bad UNARY MUL in adrput" );
		}
		return;

	default:
		cerror( "illegal address" );
		return;

		}

	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){	/* constant only */
		printf( CONFMT, p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {	/* name only */
		printf( "%.8s", p->in.name );
		}
	else {				/* name + offset */
		printf( "%.8s+", p->in.name );
		printf( CONFMT, p->tn.lval );
		}
	}

genscall( p, cookie ) register NODE *p; {
	/* structure valued call */
	return( gencall( p, cookie ) );
	}

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p */
	register temp;
	register m;

	if( p->in.right ) temp = argsize( p->in.right );
	else temp = 0;

	if( p->in.right ){ /* generate args */
		genargs( p->in.right );
		}

	if( !shltype( p->in.left->in.op, p->in.left ) ) {
		order( p->in.left, INBREG|SOREG );
		}

	p->in.op = UNARY CALL;
	m = match( p, INTAREG|INTBREG );
	popargs( temp );
	return(m != MDONE);
	}

popargs( size ) register size; {
	/* pop arguments from stack */

	toff -= size;
	if( size > 0 ){
		printf( "\tadd\tsp,#%d\n", size);
		}
	}

char *
ccbranches[] = {
	"	jr eq,.L%d\n",
	"	jr ne,.L%d\n",
	"	jr le,.L%d\n",
	"	jr lt,.L%d\n",
	"	jr ge,.L%d\n",
	"	jr gt,.L%d\n",
	"	jr ule,.L%d\n",
	"	jr ult,.L%d\n",
	"	jr uge,.L%d\n",
	"	jr ugt,.L%d\n",
	};

/*	long branch table

   This table, when indexed by a logical operator,
   selects a set of three logical conditions required
   to generate long comparisons and branches.  A zero
   entry indicates that no branch is required.
   E.G.:  The <= operator would generate:
	cp	AL,AR  (high words)
	jr lt	lable	/ 1st entry LT -> lable
	jr gt	1f	/ 2nd entry GT -> 1f
	cp	UL,UR  (low words)
	jr ule	lable	/ 3rd entry ULE -> lable
   1:
 */

int lbranches[][3] = {
	/*EQ*/	0,	NE,	EQ,
	/*NE*/	NE,	0,	NE,
	/*LE*/	LT,	GT,	ULE,
	/*LT*/	LT,	GT,	ULT,
	/*GE*/	GT,	LT,	UGE,
	/*GT*/	GT,	LT,	UGT,
	/*ULE*/	ULT,	UGT,	ULE,
	/*ULT*/	ULT,	UGT,	ULT,
	/*UGE*/	UGT,	ULT,	UGE,
	/*UGT*/	UGT,	ULT,	UGT,
	};

/* logical relations when compared in reverse order (cmp R,L) */
extern short revrel[] ;

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */
	register *plb;
	int lab1f;

	if( o == 0 ) printf( "	jr	.L%d\n", lab );
	else	if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else {
		switch( brcase ) {

		case 'A':
		case 'C':
			plb = lbranches[ o-EQ ];
			lab1f = getlab();
			expand( brnode, FORCC, brcase=='C' ? "\tcp\tAL,AR\n" : "\ttest\tAR\n" );
			if( *plb != 0 )
				printf( ccbranches[*plb-EQ], lab);
			if( *++plb != 0 )
				printf( ccbranches[*plb-EQ], lab1f);
			expand( brnode, FORCC, brcase=='C' ? "\tcp\tUL,UR\n" : "\ttest\tUR\n" );
			printf( ccbranches[*++plb-EQ], lab);
			deflab( lab1f );
			/* reclaim handled by match() after expand returns */
			break;

		default:
			if( mode=='F' ) o = revrel[ o-EQ ];
			printf( ccbranches[o-EQ], lab );
			break;
			}

		brcase = 0;
		brnode = 0;
		}
	}

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}

struct functbl {
	int fop;
	TWORD ftype;
	char *func;
	} opfunc[] = {
	MUL,		LONG,	"lmul",
	DIV,		LONG,	"ldiv",
	MOD,		LONG,	"lrem",
	ASG MUL,	LONG,	"almul",
	ASG DIV,	LONG,	"aldiv",
	ASG MOD,	LONG,	"alrem",
	MUL,		ULONG,	"ulmul",
	DIV,		ULONG,	"uldiv",
	MOD,		ULONG,	"ulrem",
	ASG MUL,	ULONG,	"aulmul",
	ASG DIV,	ULONG,	"auldiv",
	ASG MOD,	ULONG,	"aulrem",
	PLUS,		DOUBLE,	"fadd",
	MINUS,		DOUBLE, "fsub",
	MUL,		DOUBLE, "fmul",
	DIV,		DOUBLE, "fdiv",
	UNARY MINUS,	DOUBLE, "fneg",
	UNARY MINUS,	FLOAT,	"fnegf",
	ASG PLUS,	DOUBLE,	"afadd",
	ASG MINUS,	DOUBLE, "afsub",
	ASG MUL,	DOUBLE, "afmul",
	ASG DIV,	DOUBLE, "afdiv",
	ASG PLUS,	FLOAT,	"afaddf",
	ASG MINUS,	FLOAT,	"afsubf",
	ASG MUL,	FLOAT,	"afmulf",
	ASG DIV,	FLOAT,	"afdivf",
	PLUS,		FLOAT,	"faddf",
	MINUS,		FLOAT,	"fsubf",
	MUL,		FLOAT,	"fmulf",
	DIV,		FLOAT,	"fdivf",
	INCR, DOUBLE, "dfpost",
	DECR, DOUBLE, "dfpost",
	INCR, FLOAT, "ffpost",
	DECR, FLOAT, "ffpost",
	0,	0,	0 };

hardops(p)  register NODE *p; {
	/* change hard to do operators into function calls. */
	register NODE *q;
	register struct functbl *f;
	register o;
	register TWORD t;

	o = p->in.op;
	t = p->in.type;

	if (o==SCONV) { hardconv(p); return; }

	for( f=opfunc; f->fop; f++ ) {
		if( o==f->fop && t==f->ftype ) goto convert;
		}
	return;

	/* need address of left node for ASG OP */
	/* WARNING - this won't work for long in a REG */
	convert:
	if((o==INCR || o==DECR) && (t==FLOAT || t==DOUBLE)) {
		tfree(p->in.right); p->in.right=q=talloc();
		q->in.op=ICON; q->in.type=INT; q->in.rall=NOPREF;
		q->tn.lval=o==INCR?1:-1; q->tn.rval=0; q->tn.name[0]=0;
	}

	if( asgop( o ) ) {
		switch( p->in.left->in.op ) {

		case UNARY MUL:	/* convert to address */
			p->in.left->in.op = FREE;
			p->in.left = p->in.left->in.left;
			break;

		case NAME:	/* convert to ICON pointer */
			p->in.left->in.op = ICON;
			p->in.left->in.type = INCREF( p->in.left->in.type );
			break;

		case OREG:	/* convert OREG to address */
			p->in.left->in.op = REG;
			p->in.left->in.type = INCREF( p->in.left->in.type );
			if( p->in.left->tn.lval != 0 ) {
				q = talloc();
				q->in.op = PLUS;
				q->in.rall = NOPREF;
				q->in.type = p->in.left->in.type;
				q->in.left = p->in.left;
				q->in.right = talloc();

				q->in.right->in.op = ICON;
				q->in.right->in.rall = NOPREF;
				q->in.right->in.type = INT;
				q->in.right->in.name[0] = '\0';
				q->in.right->tn.lval = p->in.left->tn.lval;
				q->in.right->tn.rval = 0;

				p->in.left->tn.lval = 0;
				p->in.left = q;
				}
			break;

		/* rewrite "foo <op>= bar" as "foo = foo <op> bar" for foo in a reg */
		case REG:
			q = talloc();
			q->in.op = p->in.op - 1;	/* change <op>= to <op> */
			q->in.rall = p->in.rall;
			q->in.type = p->in.type;
			q->in.left = talloc();
			q->in.right = p->in.right;
			p->in.op = ASSIGN;
			p->in.right = q;
			q = q->in.left;			/* make a copy of "foo" */
			q->in.op = p->in.left->in.op;
			q->in.rall = p->in.left->in.rall;
			q->in.type = p->in.left->in.type;
			q->tn.lval = p->in.left->tn.lval;
			q->tn.rval = p->in.left->tn.rval;
			hardops(p->in.right);
			return;

		default:
			cerror( "Bad address for hard ops" );
			/* NO RETURN */

			}
		}

	/* build comma op for args to function */
	if ( optype(p->in.op) == BITYPE ) {
	  q = talloc();
	  q->in.op = CM;
	  q->in.rall = NOPREF;
	  q->in.type = INT;
	  q->in.left = p->in.left;
	  q->in.right = p->in.right;
	} else q = p->in.left;

	p->in.op = CALL;
	p->in.right = q;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
	strcpy( q->in.name, f->func );
	q->tn.lval = 0;
	q->tn.rval = 0;

	return;

	}

/* Internal helpers use the actual target widths, without user-call promotions. */
static NODE *fpconv(p,t)
NODE *p;
TWORD t;
{
	NODE *q;
	if(p->in.type==t) return p;
	q=block(SCONV,p,NIL,t,0,(int)t);
	hardconv(q);
	return q;
}

hardconv(p)
NODE *p;
{
	NODE *q;
	TWORD t=p->in.type, tl=p->in.left->in.type;
	char *name;
	if(t==tl || ((t!=FLOAT && t!=DOUBLE) && (tl!=FLOAT && tl!=DOUBLE))) return;
	if(tl==FLOAT && t!=DOUBLE) { p->in.left=fpconv(p->in.left,DOUBLE); tl=DOUBLE; }
	if(t==FLOAT && tl!=DOUBLE) { p->in.left=fpconv(p->in.left,DOUBLE); tl=DOUBLE; }
	if(t==FLOAT) name="dtof";
	else if(tl==FLOAT) name="ftod";
	else if(t==DOUBLE) {
		if(tl==LONG) name="ltod";
		else if(tl==ULONG) name="ultod";
		else {
			int u=ISUNSIGNED(tl) || ISPTR(tl);
			p->in.left=makety(p->in.left,u?UNSIGNED:INT,0,u?UNSIGNED:INT);
			name=u?"utod":"itod";
		}
	} else {
		if(t==LONG) name="dtol";
		else if(t==ULONG) name="dtoul";
		else name=(ISUNSIGNED(t)||ISPTR(t))?"dtou":"dtoi";
	}
	p->in.op=CALL; p->in.right=p->in.left;
	p->in.left=q=talloc();
	q->in.op=ICON; q->in.rall=NOPREF; q->in.type=INCREF(FTN+t);
	strcpy(q->tn.name,name); q->tn.lval=q->tn.rval=0;
}

optim2(p)
NODE *p;
{
	NODE *q,*args;
	char *name;
	int o=p->in.op;
	if(o>=EQ && o<=GT &&
	   (p->in.left->in.type==FLOAT || p->in.left->in.type==DOUBLE)) {
		switch(o) {
		case EQ: name="feq"; break; case NE: name="fne"; break;
		case LT: name="flt"; break; case LE: name="fle"; break;
		case GT: name="fgt"; break; default: name="fge";
		}
		args=block(CM,fpconv(p->in.left,DOUBLE),fpconv(p->in.right,DOUBLE),INT,0,INT);
		p->in.op=CALL; p->in.type=INT; p->in.right=args;
		p->in.left=q=talloc(); q->in.op=ICON; q->in.type=INCREF(FTN+INT);
		q->in.rall=NOPREF; q->tn.lval=q->tn.rval=0; strcpy(q->tn.name,name);
	}
}

myreader(p) register NODE *p; {
	walkf( p, optim2 );
	walkf( p, hardops );	/* convert ops to function calls */
	canon( p );		/* expands r-vals for fields */
	toff = 0;  /* stack offset swindle */
	}

special( p, shape ) register NODE *p; {
	/* special shape matching routine */

	switch( shape ) {

	case SCCON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= -128 && p->tn.lval <=127 ) return( 1 );
		break;

	case SICON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= 0 && p->tn.lval <=32767 ) return( 1 );
		break;

	case S16CON:
		if( p->in.op == ICON && p->in.name[0]=='\0' && p->tn.lval>= 1 && p->tn.lval <= 16) return( 1 );
		break;

	default:
		cerror( "bad special shape" );

		}

	return( 0 );
	}

# ifndef ONEPASS
main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}
# endif

fpword(p,k) NODE *p; int k; {
	CONSZ save = p->tn.lval;
	int r = p->tn.rval;
	if(p->in.op==REG) {
		p->tn.rval += k;
	} else if(p->in.op==UNARY MUL) {
		NODE *q = p->in.left;
		if(k) printf("%d(%s)",2*k,rnames[q->tn.rval]);
		else printf("@%s",rnames[q->tn.rval]);
		usedregs |= 1<<q->tn.rval;
		return;
	} else p->tn.lval += 2*k;
	adrput(p);
	p->tn.lval = save;
	p->tn.rval = r;
}
