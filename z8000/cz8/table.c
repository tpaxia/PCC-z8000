# include "mfile2"

# define TSCALAR TCHAR|TUCHAR|TSHORT|TUSHORT|TINT|TUNSIGNED|TPOINT
# define TWORD TINT|TUNSIGNED|TSHORT|TUSHORT|TPOINT
# define EA SNAME|SOREG|SCON|SAREG|SBREG|STARNM
# define EAA SNAME|SOREG|SAREG|STARNM

struct optab  table[] = {

/* IEEE binary64 values occupy four consecutive word registers. */
ASSIGN, INAREG|FOREFF,
	SAREG, TDOUBLE, EAA, TDOUBLE,
	0, RLEFT|RRIGHT, "ZX",
ASSIGN, INAREG|FOREFF,
	SNAME|SOREG|STARNM, TDOUBLE, SAREG, TDOUBLE,
	0, RLEFT|RRIGHT, "ZX",
ASSIGN, INAREG|FOREFF,
	SNAME|SOREG|STARNM, TDOUBLE, SNAME|SOREG|STARNM, TDOUBLE,
	NAREG, RLEFT|RRIGHT, "ZX",
OPLTYPE, INTAREG|INAREG,
	SANY, TANY, EAA, TDOUBLE,
	NAREG|NASR, RESC1, "ZE",
OPLTYPE, FORARG,
	SANY, TANY, EAA, TDOUBLE,
	0, RNULL, "ZG",
OPLTYPE, INTEMP,
	SANY, TANY, SAREG, TDOUBLE,
	4*NTEMP, RESC1, "ZK",

/* IEEE binary64 values occupy four consecutive word registers. */
ASSIGN, INAREG|FOREFF,
	SAREG, TFLOAT, EAA, TFLOAT,
	0, RLEFT|RRIGHT, "ZX",
ASSIGN, INAREG|FOREFF,
	SNAME|SOREG|STARNM, TFLOAT, SAREG, TFLOAT,
	0, RLEFT|RRIGHT, "ZX",
ASSIGN, INAREG|FOREFF,
	SNAME|SOREG|STARNM, TFLOAT, SNAME|SOREG|STARNM, TFLOAT,
	NAREG, RLEFT|RRIGHT, "ZX",
OPLTYPE, INTAREG|INAREG,
	SANY, TANY, EAA, TFLOAT,
	NAREG|NASR, RESC1, "ZE",
OPLTYPE, FORARG,
	SANY, TANY, EAA, TFLOAT,
	0, RNULL, "ZG",
OPLTYPE, INTEMP,
	SANY, TANY, SAREG, TFLOAT,
	2*NTEMP, RESC1, "ZK",

OPLTYPE, FORCC,
	SANY, TANY, EAA, TDOUBLE|TFLOAT,
	NAREG, RESCC, "ZY",

/* === ASSIGN === */

/* clear word/byte to zero */
ASSIGN,	INAREG|FOREFF,
	EAA,	TSCALAR,
	SZERO,	TANY,
		0,	RLEFT|RRIGHT,
		"	clrZB	AL\n",

/* clear long to zero - pair of clears */
ASSIGN,	INAREG|FOREFF,
	EAA,	TLONG|TULONG,
	SZERO,	TANY,
		0,	RLEFT|RRIGHT,
		"	clr	AL\n	clr	UL\n",

/* assign word/byte: reg = anything */
ASSIGN,	INAREG|FOREFF,
	SAREG|STAREG,	TSCALAR,
	EA,	TSCALAR,
		0,	RLEFT|RRIGHT,
		"	ldZB	AL,AR\n",

/* assign word/byte: mem/starnm = reg */
ASSIGN,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TSCALAR,
	SAREG|SBREG,	TSCALAR,
		0,	RLEFT|RRIGHT,
		"	ldZB	AL,AR\n",

/* assign word/byte: mem = mem (through temp register) */
ASSIGN,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TSCALAR,
	SNAME|SOREG|STARNM,	TSCALAR,
		NAREG,	RLEFT|RRIGHT,
		"	ldZB	A1,AR\n	ldZB	AL,A1\n",

/* assign word: breg = anything */
ASSIGN, INBREG|FOREFF,
	SBREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT|RRIGHT,
		"	ld	AL,AR\n",

/* assign long: reg = reg (word by word) */
ASSIGN,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SAREG,	TLONG|TULONG,
		0,	RLEFT|RRIGHT,
		"	ld	AL,AR\n	ld	UL,UR\n",

/* assign long: reg = mem */
ASSIGN,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		0,	RLEFT|RRIGHT,
		"	ld	AL,AR\n	ld	UL,UR\n",

/* assign long: mem = reg */
ASSIGN,	INAREG|FOREFF,
	SNAME|SOREG,	TLONG|TULONG,
	SAREG,	TLONG|TULONG,
		0,	RLEFT|RRIGHT,
		"	ld	AL,AR\n	ld	UL,UR\n",

/* assign long: reg = const (word by word) */
ASSIGN,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SCON,	TLONG|TULONG,
		0,	RLEFT|RRIGHT,
		"	ld	AL,AR\n	ld	UL,UR\n",

/* assign to bit field: clear field */
/* Z8000 has no memory-dest AND, so load/and/store through temp reg */
ASSIGN, INAREG|FOREFF,
	SFLD,	TANY,
	SZERO,	TANY,
		NAREG,	RRIGHT,
		"	ld	A1,AL\n	and	A1,#N\n	ld	AL,A1\n",

/* assign to bit field: general case */
/* Z8000 has no memory-dest AND/OR, so load/modify/store through A1 */
ASSIGN, INTAREG|INAREG|FOREFF,
	SFLD,	TANY,
	SAREG|STAREG,	TANY,
		NAREG,	RRIGHT,
		"F	push	@sp,AR\n	ld	A1,#H\n	sda	AR,A1\n	and	AR,#M\n	ld	A1,AL\n	and	A1,#N\n	or	A1,AR\n	ld	AL,A1\nF	pop	AR,@sp\n",

/* === UNARY MUL (indirect load) === */

/* indirect load word through address register */
UNARY MUL,	INTAREG|INAREG,
	SBREG,	TWORD,
	SANY,	TANY,
		NAREG|NASR,	RESC1,
		"	ld	A1,@AL\n",

/* indirect load byte through address register */
UNARY MUL,	INTAREG|INAREG,
	SBREG,	TCHAR|TUCHAR,
	SANY,	TANY,
		NAREG|NASR,	RESC1,
		"	ldb	A1,@AL\n",

/* indirect load word through data register (must not be r0) */
UNARY MUL,	INTAREG|INAREG,
	SAREG,	TWORD,
	SANY,	TANY,
		NAREG|NASR,	RESC1,
		"	ld	A1,@AL\n",

/* indirect load byte through data register */
UNARY MUL,	INTAREG|INAREG,
	SAREG,	TCHAR|TUCHAR,
	SANY,	TANY,
		NAREG|NASR,	RESC1,
		"	ldb	A1,@AL\n",

/* indirect load into breg */
UNARY MUL,	INTBREG|INBREG,
	SBREG,	TWORD,
	SANY,	TANY,
		NBREG|NBSR,	RESC1,
		"	ld	A1,@AL\n",

/* === OPLTYPE (load) === */

/* throw away useless computation */
OPLTYPE,	FOREFF,
	SNAME|SOREG|SCON|SAREG|SBREG,	TANY,
	EA,	TANY,
		0,	RRIGHT,
		"",

/* test for condition code */
OPLTYPE,	FORCC,
	SANY,	TANY,
	SAREG,	TWORD,
		0,	RESCC,
		"	test	AR\n",

OPLTYPE,	FORCC,
	SANY,	TANY,
	SAREG,	TCHAR|TUCHAR,
		0,	RESCC,
		"	testb	AR\n",

OPLTYPE,	FORCC,
	SANY,	TANY,
	SBREG,	TWORD,
		0,	RESCC,
		"	test	AR\n",

OPLTYPE,	FORCC,
	SANY,	TANY,
	SNAME|SOREG,	TWORD,
		0,	RESCC,
		"	test	AR\n",

OPLTYPE,	FORCC,
	SANY,	TANY,
	SNAME|SOREG,	TCHAR|TUCHAR,
		0,	RESCC,
		"	testb	AR\n",

/* test long in temp register pair: or halves to set Z flag */
OPLTYPE,	FORCC,
	SANY,	TANY,
	STAREG,	TLONG|TULONG,
		0,	RESCC,
		"	or	AR,UR\n",

/* test long in memory: load high, or with low */
OPLTYPE,	FORCC,
	SANY,	TANY,
	SNAME|SOREG,	TLONG|TULONG,
		NAREG,	RESCC,
		"	ld	A1,AR\n	or	A1,UR\n",

/* load zero into register */
OPLTYPE,	INTAREG|INAREG,
	SANY,	TANY,
	SZERO,	TSCALAR,
		NAREG|NASR,	RESC1,
		"	clrZB	A1\n",

/* load word/byte value into data register */
OPLTYPE,	INTAREG|INAREG,
	SANY,	TANY,
	EA,	TSCALAR,
		NAREG|NASR,	RESC1,
		"	ldZB	A1,AR\n",

/* load long value into data register (word by word) */
OPLTYPE,	INTAREG|INAREG,
	SANY,	TANY,
	EA,	TLONG|TULONG,
		NAREG|NASR,	RESC1,
		"	ld	A1,AR\n	ld	U1,UR\n",

/* load word into address register */
OPLTYPE,	INTBREG|INBREG,
	SANY,	TANY,
	EA,	TWORD,
		NBREG|NBSR,	RESC1,
		"	ld	A1,AR\n",

/* load into temp from register or constant */
OPLTYPE,	INTEMP,
	SANY,	TANY,
	SAREG|SBREG|SCON,	TSCALAR,
		NTEMP,	RESC1,
		"	ldZB	A1,AR\n",

/* load word into temp from memory (Z8000 can't do mem-to-mem ld) */
OPLTYPE,	INTEMP,
	SANY,	TANY,
	SNAME|SOREG|STARNM,	TWORD,
		NTEMP,	RESC1,
		"	ld	r0,AR\n	ld	A1,r0\n",

/* load byte into temp from memory */
OPLTYPE,	INTEMP,
	SANY,	TANY,
	SNAME|SOREG|STARNM,	TCHAR|TUCHAR,
		NTEMP,	RESC1,
		"	ldb	rl0,AR\n	ldb	A1,rl0\n",

/* load long into temp from register or constant */
OPLTYPE,	INTEMP,
	SANY,	TANY,
	SAREG|SCON,	TLONG|TULONG,
		NTEMP+NTEMP,	RESC1,
		"	ld	A1,AR\n	ld	U1,UR\n",

/* load long into temp from memory (Z8000 can't do mem-to-mem ld) */
OPLTYPE,	INTEMP,
	SANY,	TANY,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		NTEMP+NTEMP,	RESC1,
		"	ld	r0,AR\n	ld	A1,r0\n	ld	r0,UR\n	ld	U1,r0\n",

/* push word argument - register source */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SAREG|SBREG,	TWORD,
		0,	RNULL,
		"	push	@sp,AR\nZ-",

/* push word argument - constant source */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SCON,	TWORD,
		0,	RNULL,
		"	push	@sp,AR\nZ-",

/* push word argument - memory source (load then push) */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SNAME|SOREG|STARNM,	TWORD,
		0,	RNULL,
		"	ld	r0,AR\n	push	@sp,r0\nZ-",

/* push char argument - sign extend to word then push */
OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TCHAR,
		0,	RNULL,
		"	ldb	rl0,AR\n	extsb	r0\n	push	@sp,r0\nZ-",

/* push unsigned char argument - zero extend to word then push */
OPLTYPE,	FORARG,
	SANY,	TANY,
	EA,	TUCHAR,
		0,	RNULL,
		"	clr	r0\n	ldb	rl0,AR\n	push	@sp,r0\nZ-",

/* push long argument (high word first to stack, so push low then high) */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SAREG,	TLONG|TULONG,
		0,	RNULL,
		"	push	@sp,UR\n	push	@sp,AR\nZ-Z-",

/* push long argument from memory */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		0,	RNULL,
		"	ld	r0,UR\n	push	@sp,r0\n	ld	r0,AR\n	push	@sp,r0\nZ-Z-",

/* push long constant */
OPLTYPE,	FORARG,
	SANY,	TANY,
	SCON,	TLONG|TULONG,
		0,	RNULL,
		"	push	@sp,UR\n	push	@sp,AR\nZ-Z-",

/* === OPLOG (comparison) === */

/* compare word: reg vs anything */
OPLOG,	FORCC,
	SAREG|STAREG|SBREG|STBREG,	TWORD,
	EA,	TWORD,
		0,	RESCC,
		"	cp	AL,AR\nZI",

/* compare word: mem vs const */
OPLOG,	FORCC,
	SNAME|SOREG|SAREG|SBREG,	TWORD,
	SCON,	TWORD,
		0,	RESCC,
		"	cp	AL,AR\nZI",

/* compare byte: reg vs anything */
OPLOG,	FORCC,
	SAREG|STAREG,	TCHAR|TUCHAR,
	EA,	TCHAR|TUCHAR,
		0,	RESCC,
		"	cpb	AL,AR\nZI",

/* compare long: reg vs reg (word by word via cbgen) */
OPLOG,	FORCC,
	SAREG|STAREG,	TLONG|TULONG,
	SAREG,	TLONG|TULONG,
		0,	RESCC,
		"ZLZI",

/* compare long: reg vs mem */
OPLOG,	FORCC,
	SAREG|STAREG,	TLONG|TULONG,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		0,	RESCC,
		"ZLZI",

/* compare long: reg vs const */
OPLOG,	FORCC,
	SAREG|STAREG,	TLONG|TULONG,
	SCON,	TLONG|TULONG,
		0,	RESCC,
		"ZLZI",

/* === CCODES (set register to 0/1 based on condition) === */

CCODES,	INTAREG|INAREG,
	SANY,	TANY,
	SANY,	TANY,
		NAREG,	RESC1,
		"	ld	A1,#1\nZN",

/* === UNARY MINUS === */

UNARY MINUS,	INTAREG|INAREG,
	STAREG,	TWORD,
	SANY,	TANY,
		0,	RLEFT,
		"	neg	AL\n",

UNARY MINUS,	INTAREG|INAREG,
	STAREG,	TCHAR|TUCHAR,
	SANY,	TANY,
		0,	RLEFT,
		"	negb	AL\n",

/* long negate: com high, com low, add 1 to low, propagate carry */
UNARY MINUS,	INTAREG|INAREG,
	STAREG,	TLONG|TULONG,
	SANY,	TANY,
		NAREG,	RLEFT,
		"	com	AL\n	com	UL\n	add	UL,#1\n	clr	A1\n	adc	AL,A1\n",

/* === COMPL (bitwise complement) === */

COMPL,	INTAREG|INAREG,
	STAREG,	TWORD,
	SANY,	TANY,
		0,	RLEFT,
		"	com	AL\n",

COMPL,	INTAREG|INAREG,
	STAREG,	TCHAR|TUCHAR,
	SANY,	TANY,
		0,	RLEFT,
		"	comb	AL\n",

/* complement both words of a long register pair */
COMPL, INTAREG|INAREG,
	STAREG, TLONG|TULONG,
	SANY, TANY,
		0, RLEFT,
		"	com	AL\n	com	UL\n",

/* === INCR/DECR === */

/* post-increment with small constant (1-16) */
INCR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	S16CON,	TSCALAR,
		NAREG,	RESC1,
		"F	ldZB	A1,AL\n	incZB	AL,AR\n",

DECR,	INTAREG|INAREG|FOREFF,
	EAA,	TSCALAR,
	S16CON,	TSCALAR,
		NAREG,	RESC1,
		"F	ldZB	A1,AL\n	decZB	AL,AR\n",

/* post-increment with general constant - register dest */
INCR,	INTAREG|INAREG|FOREFF,
	SAREG|STAREG,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"F	ldZB	A1,AL\n	add	AL,AR\n",

DECR,	INTAREG|INAREG|FOREFF,
	SAREG|STAREG,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"F	ldZB	A1,AL\n	sub	AL,AR\n",

/* post-increment with general constant - memory dest, effect only */
INCR,	FOREFF,
	SNAME|SOREG|STARNM,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"	ld	A1,AL\n	add	A1,AR\n	ld	AL,A1\n",

DECR,	FOREFF,
	SNAME|SOREG|STARNM,	TSCALAR,
	SCON,	TSCALAR,
		NAREG,	RESC1,
		"	ld	A1,AL\n	sub	A1,AR\n	ld	AL,A1\n",

/* post-increment with general constant - memory dest, need old value */
INCR,	INTAREG|INAREG,
	SNAME|SOREG|STARNM,	TSCALAR,
	SCON,	TSCALAR,
		NAREG+NAREG,	RESC1,
		"	ld	A1,AL\n	ld	A2,A1\n	add	A2,AR\n	ld	AL,A2\n",

DECR,	INTAREG|INAREG,
	SNAME|SOREG|STARNM,	TSCALAR,
	SCON,	TSCALAR,
		NAREG+NAREG,	RESC1,
		"	ld	A1,AL\n	ld	A2,A1\n	sub	A2,AR\n	ld	AL,A2\n",

/* post-increment into breg */
INCR,	INTBREG|INBREG|FOREFF,
	SBREG,	TWORD,
	S16CON,	TWORD,
		NBREG,	RESC1,
		"F	ld	A1,AL\n	inc	AL,AR\n",

DECR,	INTBREG|INBREG|FOREFF,
	SBREG,	TWORD,
	S16CON,	TWORD,
		NBREG,	RESC1,
		"F	ld	A1,AL\n	dec	AL,AR\n",

INCR,	INTBREG|INBREG|FOREFF,
	SBREG,	TWORD,
	SCON,	TWORD,
		NBREG,	RESC1,
		"F	ld	A1,AL\n	add	AL,AR\n",

DECR,	INTBREG|INBREG|FOREFF,
	SBREG,	TWORD,
	SCON,	TWORD,
		NBREG,	RESC1,
		"F	ld	A1,AL\n	sub	AL,AR\n",

/* === ASG PLUS / ASG MINUS === */

/* add/sub with small constant (1-16): inc/dec */
ASG PLUS,	INAREG|FOREFF,
	EAA,	TWORD,
	S16CON,	TWORD,
		0,	RLEFT,
		"	inc	AL,AR\n",

ASG MINUS,	INAREG|FOREFF,
	EAA,	TWORD,
	S16CON,	TWORD,
		0,	RLEFT,
		"	dec	AL,AR\n",

/* add/sub: reg op= src */
ASG PLUS,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	add	AL,AR\n",

ASG PLUS,	INBREG|FOREFF,
	SBREG|STBREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	add	AL,AR\n",

ASG MINUS,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	sub	AL,AR\n",

ASG MINUS,	INBREG|FOREFF,
	SBREG|STBREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	sub	AL,AR\n",

/* add/sub byte */
ASG PLUS,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR|TUCHAR,
	EA,	TCHAR|TUCHAR,
		0,	RLEFT,
		"	addb	AL,AR\n",

ASG MINUS,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR|TUCHAR,
	EA,	TCHAR|TUCHAR,
		0,	RLEFT,
		"	subb	AL,AR\n",

/* add/sub: mem op= src (Z8000 add/sub are reg-dest only, use load/op/store) */
ASG PLUS,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TWORD,
	EA,	TWORD,
		NAREG,	RLEFT,
		"	ld	A1,AL\n	add	A1,AR\n	ld	AL,A1\n",

ASG MINUS,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TWORD,
	EA,	TWORD,
		NAREG,	RLEFT,
		"	ld	A1,AL\n	sub	A1,AR\n	ld	AL,A1\n",

/* add/sub long: reg op= reg (word by word with carry) */
ASG PLUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SAREG,	TLONG|TULONG,
		0,	RLEFT,
		"	add	UL,UR\n	adc	AL,AR\n",

ASG MINUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SAREG,	TLONG|TULONG,
		0,	RLEFT,
		"	sub	UL,UR\n	sbc	AL,AR\n",

/* add/sub long: reg op= mem (adc/sbc are reg-only, need temp for high word) */
ASG PLUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		NAREG,	RLEFT,
		"	add	UL,UR\n	ld	A1,AR\n	adc	AL,A1\n",

ASG MINUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SNAME|SOREG|STARNM,	TLONG|TULONG,
		NAREG,	RLEFT,
		"	sub	UL,UR\n	ld	A1,AR\n	sbc	AL,A1\n",

/* add/sub long: reg op= const (adc/sbc are reg-only, need temp for high word) */
ASG PLUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SCON,	TLONG|TULONG,
		NAREG,	RLEFT,
		"	add	UL,UR\n	ld	A1,AR\n	adc	AL,A1\n",

ASG MINUS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SCON,	TLONG|TULONG,
		NAREG,	RLEFT,
		"	sub	UL,UR\n	ld	A1,AR\n	sbc	AL,A1\n",

/* === ASG OR / ASG AND / ASG ER (exclusive or) === */

/* and long: reg op= src (word by word) */
ASG AND,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	EA,	TLONG|TULONG,
		0,	RLEFT,
		"	and	AL,AR\n	and	UL,UR\n",

/* or long: reg op= src (word by word) */
ASG OR,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	EA,	TLONG|TULONG,
		0,	RLEFT,
		"	or	AL,AR\n	or	UL,UR\n",

/* xor long: reg op= src (word by word) */
ASG ER,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	EA,	TLONG|TULONG,
		0,	RLEFT,
		"	xor	AL,AR\n	xor	UL,UR\n",

/* xor must be separate since Z8000 xor syntax differs */
/* xor: reg op= src */
ASG ER,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	xor	AL,AR\n",

/* xor: mem op= src (Z8000 has no mem-dest xor, use load/op/store) */
ASG ER,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TWORD,
	SCON|SAREG|STAREG,	TWORD,
		NAREG,	RLEFT,
		"	ld	A1,AL\n	xor	A1,AR\n	ld	AL,A1\n",

/* or/and: reg op= src */
ASG OPSIMP,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	EA,	TWORD,
		0,	RLEFT,
		"	OI	AL,AR\n",

/* or/and: mem op= src (Z8000 has no mem-dest and/or, use load/op/store) */
ASG OPSIMP,	INAREG|FOREFF,
	SNAME|SOREG|STARNM,	TWORD,
	SCON|SAREG|STAREG,	TWORD,
		NAREG,	RLEFT,
		"	ld	A1,AL\n	OI	A1,AR\n	ld	AL,A1\n",

/* byte variants */
ASG ER,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR|TUCHAR,
	EA,	TCHAR|TUCHAR,
		0,	RLEFT,
		"	xorb	AL,AR\n",

ASG OPSIMP,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR|TUCHAR,
	EA,	TCHAR|TUCHAR,
		0,	RLEFT,
		"	OIb	AL,AR\n",

/* === ASG LS / ASG RS (shifts) === */

/* word shift by immediate count */
ASG LS,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	SCON,	TANY,
		0,	RLEFT,
		"	sla	AL,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TINT|TSHORT,
	SCON,	TANY,
		0,	RLEFT,
		"	sra	AL,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	SCON,	TANY,
		0,	RLEFT,
		"	srl	AL,AR\n",

/* word shift by register count (dynamic shift) */
ASG LS,	INAREG|FOREFF,
	SAREG|STAREG,	TWORD,
	SAREG,	TANY,
		0,	RLEFT,
		"	sda	AL,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TINT|TSHORT,
	SAREG,	TANY,
		NAREG,	RLEFT,
		"	ld	A1,AR\n	neg	A1\n	sda	AL,A1\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	SAREG,	TANY,
		NAREG,	RLEFT,
		"	ld	A1,AR\n	neg	A1\n	sdl	AL,A1\n",

/* long shift by immediate count */
ASG LS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SCON,	TANY,
		0,	RLEFT,
		"	slal	ZQ,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG,
	SCON,	TANY,
		0,	RLEFT,
		"	sral	ZQ,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TULONG,
	SCON,	TANY,
		0,	RLEFT,
		"	srll	ZQ,AR\n",

/* long shift by register count (dynamic) */
ASG LS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG|TULONG,
	SAREG,	TANY,
		0,	RLEFT,
		"	sdal	ZQ,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TLONG,
	SAREG,	TANY,
		NAREG,	RLEFT,
		"	ld	A1,AR\n	neg	A1\n	sdal	ZQ,A1\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TULONG,
	SAREG,	TANY,
		NAREG,	RLEFT,
		"	ld	A1,AR\n	neg	A1\n	sdll	ZQ,A1\n",

/* byte shift */
ASG LS,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR|TUCHAR,
	SCON,	TANY,
		0,	RLEFT,
		"	slab	AL,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TCHAR,
	SCON,	TANY,
		0,	RLEFT,
		"	srab	AL,AR\n",

ASG RS,	INAREG|FOREFF,
	SAREG|STAREG,	TUCHAR,
	SCON,	TANY,
		0,	RLEFT,
		"	srlb	AL,AR\n",

/* Byte shifts also accept a live register count. */
ASG LS, INAREG|FOREFF,
	SAREG|STAREG, TCHAR|TUCHAR, SAREG, TANY,
	0, RLEFT, "\tsdab\tAL,AR\n",
ASG RS, INAREG|FOREFF,
	SAREG|STAREG, TCHAR, SAREG, TANY,
	NAREG, RLEFT, "\tld\tA1,AR\n\tneg\tA1\n\tsdab\tAL,A1\n",
ASG RS, INAREG|FOREFF,
	SAREG|STAREG, TUCHAR, SAREG, TANY,
	NAREG, RLEFT, "\tld\tA1,AR\n\tneg\tA1\n\tsdlb\tAL,A1\n",

/* === MUL / DIV / MOD (16-bit hardware operations) === */
/* rallo ensures left operand is in r1 (part of pair rr0) */

/* signed multiply: move AL to r1, r1 * src -> rr0, result in r1 */
MUL,	INAREG|INTAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	mult	rr0,AR\nZH",

/* unsigned multiply */
MUL,	INAREG|INTAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	mult	rr0,AR\nZH",

/* signed divide: move AL to r1, sign extend, divide */
DIV,	INAREG|INTAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	exts	rr0\n	div	rr0,AR\nZH",

/* unsigned divide: clear r0, move AL to r1, divide */
DIV,	INAREG|INTAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"ZVZH",

/* signed modulus: move AL to r1, sign extend, divide, remainder r0 -> r1 */
MOD,	INAREG|INTAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	exts	rr0\n	div	rr0,AR\n	ld	r1,r0\nZH",

/* unsigned modulus: clear r0, move AL to r1, divide, remainder r0 -> r1 */
MOD,	INAREG|INTAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"ZVZH",

/* ASG MUL/DIV/MOD */
ASG MUL,	INAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	mult	rr0,AR\n	ld	AL,r1\nZH",

ASG MUL,	INAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	mult	rr0,AR\n	ld	AL,r1\nZH",

ASG DIV,	INAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	exts	rr0\n	div	rr0,AR\n	ld	AL,r1\nZH",

ASG DIV,	INAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"ZVZH",

ASG MOD,	INAREG,
	SAREG|STAREG,	TINT|TSHORT,
	EA,	TINT|TSHORT,
		0,	RLEFT,
		"	ld	r1,AL\n	exts	rr0\n	div	rr0,AR\n	ld	r1,r0\n	ld	AL,r1\nZH",

ASG MOD,	INAREG,
	SAREG|STAREG,	TUNSIGNED|TUSHORT,
	EA,	TUNSIGNED|TUSHORT,
		0,	RLEFT,
		"ZVZH",

/* === UNARY CALL === */

UNARY CALL,	INTAREG,
	SBREG|SNAME|SOREG|SCON,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1,
		"ZC\nZ0",

UNARY CALL,	INTAREG,
	SAREG,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1,
		"	ld	r8,AL\n	call	@r8\nZ0",

/* === SCONV (type conversions) === */

/* word to word (same size, just type change - no code needed) */
SCONV,	INTAREG,
	STAREG,	TWORD,
	SANY,	TWORD,
		0,	RLEFT,
		"",

/* long <-> ulong (same size, just signedness change - no code needed) */
SCONV,	INTAREG,
	STAREG,	TLONG|TULONG,
	SANY,	TLONG|TULONG,
		0,	RLEFT,
		"",

/* char -> int/word: sign extend byte to word */
SCONV,	INTAREG,
	STAREG,	TCHAR,
	SANY,	TWORD,
		0,	RLEFT,
		"	extsb	AL\n",

/* uchar -> int/word: zero extend byte (mask) */
SCONV,	INTAREG,
	STAREG,	TUCHAR,
	SANY,	TWORD,
		0,	RLEFT,
		"	and	AL,#255\n",

/* int -> long: sign extend word to long pair */
/* load source into low reg of pair, sign extend to fill high reg */
SCONV,	INTAREG|INAREG,
	EA|STAREG|STBREG,	TINT|TSHORT,
	SANY,	TLONG|TULONG,
		NAREG|NASR,	RESC1,
		"	ld	U1,AL\n	exts	ZD\n",

/* uint -> ulong: zero extend word to long pair */
/* clear pair, then load source into low reg */
SCONV,	INTAREG|INAREG,
	EA|STAREG|STBREG,	TUNSIGNED|TUSHORT|TPOINT,
	SANY,	TLONG|TULONG,
		NAREG|NASR,	RESC1,
		"	subl	ZD,ZD\n	ld	U1,AL\n",

/* long -> int/word: truncate (take low word) */
SCONV,	INTAREG|INAREG,
	SAREG|STAREG,	TLONG|TULONG,
	SANY,	TWORD,
		NAREG|NASR,	RESC1,
		"	ld	A1,UL\n",

/* long -> int/word from memory: adjust offset and load */
SCONV,	INTAREG|INAREG,
	SNAME|SOREG,	TLONG|TULONG,
	SANY,	TWORD,
		NAREG|NASR,	RESC1,
		"ZT	ld	A1,AL\n",

/* int/word -> char from memory: adjust offset for big-endian byte access */
SCONV,	INAREG|INTAREG,
	SNAME|SOREG,	TWORD,
	SANY,	TCHAR|TUCHAR,
		0,	RLEFT,
		"ZT",

/* int/word -> char: no code, just use low byte */
SCONV,	INAREG|INTAREG,
	EA,	TWORD,
	SANY,	TCHAR|TUCHAR,
		0,	RLEFT,
		"",

/* long -> char: take low byte of low word */
SCONV,	INTAREG|INAREG,
	SAREG|STAREG,	TLONG|TULONG,
	SANY,	TCHAR|TUCHAR,
		NAREG|NASR,	RESC1,
		"	ld	A1,UL\n",

/* uchar -> char, char -> uchar: no code */
SCONV,	INTAREG,
	STAREG,	TCHAR|TUCHAR,
	SANY,	TCHAR|TUCHAR,
		0,	RLEFT,
		"",

/* === STASG (structure assignment) === */

STASG,	FOREFF,
	SNAME|SOREG,	TANY,
	SCON|SBREG,	TANY,
		0,	RNOP,
		"ZS",

STASG,	INTBREG|INBREG,
	SNAME|SOREG,	TANY,
	STBREG,	TANY,
		0,	RRIGHT,
		"ZS",

STASG, INBREG|INTBREG,
	SNAME|SOREG,	TANY,
	SCON|SBREG,	TANY,
		NBREG,	RESC1,
		"ZS	ld	A1,AR\n",

/* === INIT (data initialization) === */

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TINT|TUNSIGNED|TPOINT|TSHORT|TUSHORT,
		0,	RNOP,
		"	.word	CL\n",

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TCHAR|TUCHAR,
		0,	RNOP,
		"	.byte	CL\n",

INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TLONG|TULONG,
		0,	RNOP,
		"ZJ",

	/* Default actions for hard trees ... */

# define DF(x) FORREW,SANY,TANY,SANY,TANY,REWRITE,x,""

UNARY MUL, DF( UNARY MUL ),

INCR, DF(INCR),

DECR, DF(INCR),

ASSIGN, DF(ASSIGN),

STASG, DF(STASG),

OPLEAF, DF(NAME),

OPLOG,	FORCC,
	SANY,	TANY,
	SANY,	TANY,
		REWRITE,	BITYPE,
		"",

OPLOG,	DF(NOT),

COMOP, DF(COMOP),

INIT, DF(INIT),

OPUNARY, DF(UNARY MINUS),


ASG OPANY, DF(ASG PLUS),

OPANY, DF(BITYPE),

FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	"help; I'm in trouble\n" };
