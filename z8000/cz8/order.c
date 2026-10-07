# include "mfile2"

int fltused = 0;
extern int toff, maxtoff;

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	return( shltype(p->in.left->in.op, p->in.left ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
	/* Z8000 has no auto-increment addressing, so always delay */

	p = p->in.left;
	if( p->in.op == UNARY MUL ) p = p->in.left;
	return( p->in.op == NAME || p->in.op == OREG || p->in.op == REG );
	}

mkadrs(p) register NODE *p; {
	register o;

	o = p->in.op;

	if( asgop(o) ){
		if( p->in.left->in.su >= p->in.right->in.su ){
			if( p->in.left->in.op == UNARY MUL ){
				if( p->in.left->in.su > 0 )
					SETSTO( p->in.left->in.left, INTEMP );
				else {
					if( p->in.right->in.su > 0 ) SETSTO( p->in.right, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->in.left, INTEMP );
				}
			}
		else SETSTO( p->in.right, INTEMP );
		}
	else {
		if( p->in.left->in.su > p->in.right->in.su ){
			SETSTO( p->in.left, INTEMP );
			}
		else {
			SETSTO( p->in.right, INTEMP );
			}
		}
	}

notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	/* offset of off, (from a register of r), if the
	/* resulting thing had type t */

	/* Z8000: R1-R15 can be base/index registers, R0 cannot */
	if ( off>=-32768 && off<=32767 && *cp=='\0' && r>=R1 && r<=SP )
	  return(0);	/* YES */
	return( 1 );  /* NO */
	}

# define max(x,y) ((x)<(y)?(y):(x))
# define min(x,y) ((x)<(y)?(x):(y))


# define ZCHAR 01
# define ZLONG 02
# define ZFLOAT 04

zum( p, zap ) register NODE *p; {
	/* zap Sethi-Ullman number for chars, longs, floats */
	/* in the case of longs, only STARNM's are zapped */
	/* ZCHAR, ZLONG, ZFLOAT are used to select the zapping */

	register su;

	su = p->in.su;

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su==0 && p->in.op!=ICON ) p->in.su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		/* need register pair for indirect load of long */
		if( p->in.op == UNARY MUL && su == 0 ) p->in.su = su = 2;
		break;

	case FLOAT:
		if( !(zap&ZFLOAT) ) break;
		/* floats need register pair (szty=2) */
		if( su == 0 ) p->in.su = su = 2;

		}

	return( su );
	}

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	   number, or local equivalent */

	register o, ty, sul, sur;
	register nr;

	ty = optype( o=p->in.op);
	nr = szty( p->in.type );
	p->in.su = 0;

	if( ty == LTYPE ) {
		/* leaf nodes: LONG/FLOAT need 2 regs */
		if( (p->in.type==FLOAT || p->in.type==DOUBLE) && p->in.op!=ICON ) p->in.su = szty(p->in.type);
		else if( (p->in.type==LONG||p->in.type==ULONG) && p->in.op!=ICON ) p->in.su = 2;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->in.su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->in.left ) ) return;

		default:
			p->in.su = max( p->in.left->in.su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		/* a long or double variable on the left is only addressed, never
		   loaded, so its own register count does not apply */
		/* likewise a double reached through a pointer: forming its address
		   takes what the pointer expression takes (and an address register),
		   not the four data registers a double value would */
		if( nr==4 && p->in.left->in.op==UNARY MUL )
			sul = p->in.left->in.left->in.su;
		if( sul==0 || optype(p->in.left->in.op)==LTYPE ){
			/* don't need to worry about the left side */
			p->in.su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				p->in.su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->in.su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->in.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->in.su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		nr = szty( p->in.left->in.type );
		sul = zum( p->in.left, ZLONG|ZCHAR|ZFLOAT );
		sur = zum( p->in.right, ZLONG|ZCHAR|ZFLOAT );
		p->in.su = min( max(sul,sur+nr), max(sur,sul+nr) );
		return;
		}

	if( asgop(o) ){
		switch( o ) {
		case INCR:
		case DECR:
			/* do as binary op */
			break;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:
		case ASG ER:
		case ASG OR:
			if( p->in.type == INT || p->in.type == UNSIGNED || ISPTR(p->in.type) ) goto asop;

		gencase:
		default:
			/* A directly addressed long needs no register to form its
			 * address.  Counting its value pair here invents a fifth
			 * data register and repeatedly spills the same memory lhs. */
			if( (p->in.type==LONG || p->in.type==ULONG) &&
			    (p->in.left->in.op==NAME || p->in.left->in.op==OREG) )
				sul = 0;
			sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
			if( sur == 0 ){
				if( sul == 0 ) p->in.su = nr;
				else p->in.su = max( sul, nr+1 );
				}
			else {
				if( sul == 0 ){
					p->in.su = max( sur, nr+nr );
					}
				else {
					p->in.su = max( sur, max( sul+nr, 1+nr+nr ) );
					}
				}
			return;
			}
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->in.su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MOD || o==MUL )
	    && p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr = fregs;
	if( o==PLUS || o==MUL || o==OR || o==ER ){
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->in.left ) || sul > sur ) goto noswap;

		lt = p->in.left->in.type;
		rt = p->in.right->in.type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR) && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->in.left->in.op == UNARY MUL && sul==0 ) goto noswap;
				if( p->in.right->in.op == UNARY MUL && p->in.left->in.op != UNARY MUL ) goto swap;
				goto noswap;
				}
			else if( p->in.left->in.op == UNARY MUL && sul == 0 ) goto noswap;
			else goto swap;  /* put long on right, unless STARNM */
			}

		if( sul == 0 && sur != 0 ){

			NODE *s;
			int ssu;

		swap:
			ssu = sul;  sul = sur; sur = ssu;
			s = p->in.left;  p->in.left = p->in.right; p->in.right = s;
			}
		}
	noswap:

	sur = zum( p->in.right, ZCHAR|ZLONG|ZFLOAT );
	if( sur == 0 ){
		/* get left value into a register, do op */
		p->in.su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->in.su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->in.op == FLD ){
		p->in.left->in.rall = p->in.rall;
		p = p->in.left;
		}

	if( p->in.op != UNARY MUL ) return;  /* no more to do */
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		p->in.rall = r;
		p = p->in.left;
		}
	if( p->in.op == PLUS && p->in.right->in.op == ICON ){
		p->in.rall = r;
		p = p->in.left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register o, type, down1, down2, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;
	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( o = p->in.op );
	type = p->in.type;


	if( type == DOUBLE || type == FLOAT ){
		if( o == FORCE ) down1 = R0|MUSTDO;
		++fltused;
		}
	else switch( o ) {
	case ASSIGN:
		down1 = NOPREF;
		down2 = down;
		break;

	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
		/* Z8000: mult/div uses RR0 (R0:R1) */
		/* keep addresses out of the way of R0,R1 */
		if(fregs == 2 ){
			/* lhs in R1, nothing else matters */
			down1 = R1|MUSTDO;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in R1, address of left in R2 */
		p->in.left->in.rall = R1|MUSTDO;
		mkrall( p->in.left, R2|MUSTDO );
		/* now, deal with right */
		if( fregs == 3 ) rallo( p->in.right, NOPREF );
		else {
			p->in.right->in.rall = R3|MUSTDO;
			mkrall( p->in.right, R3|MUSTDO );
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		/* Z8000 mult .rr0,src: R1 * src -> RR0 (result low word in R1) */
		/* Z8000 div .rr0,src: RR0 / src -> R1(quot), R0(rem) */
		rallo( p->in.left, R1|MUSTDO );

		if( fregs == 2 ){
			rallo( p->in.right, NOPREF );
			return;
			}
		/* compute addresses, stay away from R0,R1 */

		p->in.right->in.rall = (fregs==3) ? R2|MUSTDO : R3|MUSTDO ;
		mkrall( p->in.right, R2|MUSTDO );
		return;

	case CALL:
	case STASG:
	case EQ:
	case NE:
	case GT:
	case GE:
	case LT:
	case LE:
	case NOT:
	case ANDAND:
	case OROR:
		down1 = NOPREF;
		break;

	case FORCE:
		down1 = R0|MUSTDO;
		break;

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

offstar( p ) register NODE *p; {
	/* handle indirections */

	if( p->in.op == UNARY MUL ) p = p->in.left;

	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON && p->in.right->in.name[0]=='\0' &&
		    p->in.right->tn.lval>=-32768 && p->in.right->tn.lval<=32767 ){
			order( p->in.left , INTBREG|INBREG );
			return;
			}
		}
	order( p, INTBREG|INBREG );
	}

setincr( p ) NODE *p; {
	return( 0 );	/* for the moment, don't bother */
	}

niceuty( p ) register NODE *p; {
	register TWORD t;

	return( p->in.op == UNARY MUL && (t=p->in.type)!=CHAR &&
		t!= UCHAR && t!= FLOAT &&
		shumul( p->in.left) );
	}
setbin( p ) register NODE *p; {
	register NODE *r, *l;

	r = p->in.right;
	l = p->in.left;

	if( p->in.right->in.su == 0 ){ /* rhs is addressable */
		if( logop( p->in.op ) ){
			if( l->in.op == UNARY MUL && l->in.type != FLOAT && shumul( l->in.left ) ) offstar( l->in.left );
			else order( l, INAREG|INTAREG|INBREG|INTBREG|INTEMP );
			return( 1 );
			}
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->in.op ) ){
		/* relational: do both sides into regs if need be */

		if( r->in.su > l->in.su ){
			if( niceuty(r) ){
				offstar( r->in.left );
				return( 1 );
				}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
				return( 1 );
				}
			}
		if( niceuty(l) ){
			offstar( l->in.left );
			return( 1 );
			}
		else if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		else if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );
			}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
			}
		cerror( "setbin can't deal with %s", opst[p->in.op] );
		}

	/* ordinary operator */

	if( !istnode(r) && r->in.su > l->in.su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );  /* hopefully, it is addressable by now */
			}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
		}
	else {
		if( !istnode( l ) ){
			order( l, INTAREG|INTBREG );
			return( 1 );
			}
		/* rewrite */
		return( 0 );
		}
	}

setstr( p ) register NODE *p; { /* structure assignment */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INTBREG );
		return(1);
		}
	p = p->in.left;
	if( p->in.op != NAME && p->in.op != OREG ){
		if( p->in.op != UNARY MUL ) cerror( "bad setstr" );
		order( p->in.left, INTBREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* setup for assignment operator */

	if( p->in.right->in.su != 0 && p->in.right->in.op != REG ) {
		if( p->in.right->in.op == UNARY MUL )
			offstar( p->in.right->in.left );
		else
			order( p->in.right, INAREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	/* Z8000: no STARREG, only STARNM */
	if( p->in.left->in.op == UNARY MUL && !tshape( p->in.left, STARNM ) ){
		offstar( p->in.left->in.left );
		return(1);
		}
	if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
		offstar( p->in.left->in.left->in.left );
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INAREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* setup for =ops */
	register sul, sur;

	sul = p->in.left->in.su;
	sur = p->in.right->in.su;

	switch( p->in.op ){
	  case ASG PLUS:
	  case ASG MINUS:
	  case ASG OR:
	  case ASG ER:
	  case ASG AND:
		if (p->in.right->in.op != REG && sul == 0) {
		  order(p->in.right,INAREG|INTAREG);
		  return(1);
		} else break;

	  case ASG LS:
	  case ASG RS:
		if (p->in.left->in.op != REG) break;
		/* Z8000 static shifts support immediate 1-16 */
		if (p->in.right->in.op == REG ||
		    (p->in.right->in.op==ICON && p->in.right->tn.lval>=1 && p->in.right->tn.lval<=16))
		  break;
		order(p->in.right,INAREG|INTAREG);
		return(1);
	}

	if( sur == 0 ){

		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) return(0);  /* rewrite */

		/* harder; make left address, val, op, and store */

		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left );
			return( 1 );
			}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left->in.left );
			return( 1 );
			}
		return(0);  /* rewrite */
		}

	/* harder case: do right, left address, left value, op, store */

	if( p->in.right->in.op == UNARY MUL && p->in.left->in.op==REG){
		offstar( p->in.right->in.left );
		return( 1 );
		}
	if( p->in.right->in.op == REG ){
		/* make lhs addressable */
		if( sul == 0 ) return(0);
		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left );
			return( 1 );
			}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left->in.left );
			return( 1 );
			}
		return(0);
		}
	order( p->in.right, INAREG|INBREG );
	return( 1 );
	}

/* pass 2 label generation -- these become get2lab/def2lab via mfile2 #defines */
int crslab = 10000;

getlab(){
	return( crslab++ );
	}

deflab( l ){
	printf( ".L%d:\n", l );
	}

genargs( p) register NODE *p; {
	register size;

	/* generate code for the arguments */

	/*  first, do the arguments on the right (last->first) */
	while( p->in.op == CM ){
		genargs( p->in.right );
		p->in.op = FREE;
		p = p->in.left;
		}

	if( p->in.op == STARG ){ /* structure valued argument */

		size = p->stn.stsize;
		if( p->in.left->in.op == ICON ){
			/* make into a name node */
			p->in.op = FREE;
			p= p->in.left;
			p->in.op = NAME;
			}
		else {
			/* make it look beautiful... */
			p->in.op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			if( p->in.op != OREG ){
				offstar( p->in.left );
				canon( p );
				if( p->in.op != OREG ) cerror( "stuck starg" );
				}
			}

		p->tn.lval += size;  /* end of structure */
		/* push struct words onto stack in reverse order */
		/* Z8000: must load through register, then push */
		for( ; size>0; size -= 2 ){
			p->tn.lval -= 2;
			expand( p, RNOP, "\tld\tr0,AR\n\tpush\t@sp,r0\n" );
			toff += 2;
			}
		reclaim( p, RNULL, 0 );
		return;
		}

	/* ordinary case */

	order( p, FORARG );
	}

argsize( p ) register NODE *p; {
	register t;
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
		}
	if( p->in.type == DOUBLE ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if( p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+4 );
		}
	else if( p->in.op == STARG ){
		SETOFF( t, p->stn.stalign );  /* alignment */
		return( t + p->stn.stsize );  /* size */
		}
	else if( p->in.type == LONG || p->in.type == ULONG ){
		SETOFF( t, 2 );
		return( t+4 );
		}
	else {
		/* int, short, char, pointer: all pushed as 16-bit word */
		SETOFF( t, 2 );
		return( t+2 );
		}
	}
