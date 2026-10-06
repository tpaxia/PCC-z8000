#include "mical.h"
#include "inst.h"

struct blist *sdi_bound();
int makesdi();

char	Code_length;		/* Number of bytes in the current instruction*/
short	*WCode = (short *)Code;
struct oper operands[OPERANDS_MAX];	/* where all the operands go */
int	numops;			/* # of operands to the current instruction */

/*
 * Z8000 register classification helpers.
 * Register values from symbol table:
 *   0-15: word registers (r0-r15)
 *   16-23: byte registers rh0-rh7
 *   24-31: byte registers rl0-rl7
 *   32-46 (even): long register pairs rr0-rr14
 *   48-60 (mult 4): quad registers rq0-rq12
 */

/* is word register? */
wreg(r) { return(r >= 0 && r <= 15); }

/* is byte register? */
breg(r) { return(r >= 16 && r <= 31); }

/* is long register pair? */
lreg(r) { return(r >= 32 && r <= 46 && !(r & 1)); }

/* is quad register? */
qreg(r) { return(r >= 48 && r <= 60 && !(r & 3)); }

/* extract 4-bit register field from register value */
regfield(r)
{
	if (r <= 15) return(r);		/* word reg */
	if (r <= 23) return(r - 16);	/* rh0-rh7: codes 0-7 */
	if (r <= 31) return(r - 16);	/* rl0-rl7: codes 8-15 */
	if (r <= 46) return(r - 32);	/* rr: even reg number */
	return(r - 48);			/* rq: mult-4 reg number */
}

/* register field for byte instructions:
 * word registers r0-r7 map to low-byte codes 8-15 (RL) */
bregfield(r)
{
	if (r <= 7) return(r + 8);	/* word reg → RL code */
	return(regfield(r));
}

/* Check if operand is a register of the right type */
chk_wreg(op)
  struct oper *op;
  {	if (op->type_o != t_reg || !wreg(op->value_o)) {
		Prog_Error(E_REG);
		return(0);
	}
	return(1);
}

chk_breg(op)
  struct oper *op;
  {	if (op->type_o != t_reg || !breg(op->value_o)) {
		Prog_Error(E_REG);
		return(0);
	}
	return(1);
}

chk_lreg(op)
  struct oper *op;
  {	if (op->type_o != t_reg || !lreg(op->value_o)) {
		Prog_Error(E_REG);
		return(0);
	}
	return(1);
}

/* Check for indirect register operand (must be @.r1-@.r15, not @.r0) */
chk_ireg(op)
  struct oper *op;
  {	if (op->type_o != t_ireg) {
		Prog_Error(E_OPERAND);
		return(0);
	}
	if (!wreg(op->reg_o) || op->reg_o == 0) {
		Prog_Error(E_REG);
		return(0);
	}
	return(1);
}

/* Instruction  -- Z8000 assembler
 * This program is called from the main loop. It checks to see if the
 * operator field is a valid instruction mnemonic and generates machine code.
 */
Instruction(opindex)
{	register int i;

	if (Cur_csect==Text_csect && opindex<i_long && (Dot&1)) Prog_Error(E_ODDADDR);
	Code_length = 2;		/* always at least 2 bytes of code */
	for(i=0;i < CODE_MAX; i++) { Code[i] = 0; }

	switch (opindex) {

/* =================== No-operand instructions =================== */

	case i_nop:	no_op(0x8D07); break;
	case i_halt:	no_op(0x7A00); break;
	case i_iret:	no_op(0x7B00); break;
	case i_mbit:	no_op(0x7B0A); break;
	case i_mreq:	no_op(0x7B0D); break;
	case i_mres:	no_op(0x7B09); break;
	case i_mset:	no_op(0x7B08); break;
	case i_di:	di_ei(0x7C00); break;
	case i_ei:	di_ei(0x7C04); break;
	case i_sc:	sc_op(); break;
	case i_ldctl:	ldctl_op(); break;
	case i_ldctlb:	ldctlb_op(); break;

/* =================== Load instructions =================== */

	case i_ld:	ld_op(W); break;
	case i_ldb:	ld_op(B); break;
	case i_ldl:	ldl_op(); break;
	case i_lda:	lda_op(); break;
	case i_ldk:	ldk_op(); break;
	case i_ldm:	ldm_op(); break;
	case i_ldr:	ldr_op(W); break;
	case i_ldrb:	ldr_op(B); break;
	case i_ldrl:	ldr_op(L); break;
	case i_ex:	ex_op(W); break;
	case i_exb:	ex_op(B); break;

/* =================== Arithmetic instructions =================== */

	case i_add:	alu_op(0x8100, 0x0100, W, 0); break;
	case i_addb:	alu_op(0x8000, 0x0000, B, 0); break;
	case i_addl:	alul_op(0x9600); break;
	case i_adc:	rr_op(0xB500, W); break;
	case i_adcb:	rr_op(0xB400, B); break;
	case i_sub:	alu_op(0x8300, 0x0300, W, 0); break;
	case i_subb:	alu_op(0x8200, 0x0200, B, 0); break;
	case i_subl:	alul_op(0x9200); break;
	case i_sbc:	rr_op(0xB700, W); break;
	case i_sbcb:	rr_op(0xB600, B); break;

	case i_inc:	inc_dec(0x2900, W); break;
	case i_incb:	inc_dec(0x2800, B); break;
	case i_dec:	inc_dec(0x2B00, W); break;
	case i_decb:	inc_dec(0x2A00, B); break;

	case i_neg:	one_dst(0x8D02, W); break;
	case i_negb:	one_dst(0x8C02, B); break;
	case i_da:	one_reg(0xB000, W); break;
	case i_dab:	one_reg(0xB000, B); break;

/* =================== Compare instructions =================== */

	case i_cp:	alu_op(0x8B00, 0x0B00, W, 0x01); break;
	case i_cpb:	alu_op(0x8A00, 0x0A00, B, 0x01); break;
	case i_cpl:	alul_op(0x9000); break;

/* =================== Logical instructions =================== */

	case i_and:	alu_op(0x8700, 0x0700, W, 0); break;
	case i_andb:	alu_op(0x8600, 0x0600, B, 0); break;
	case i_or:	alu_op(0x8500, 0x0500, W, 0); break;
	case i_orb:	alu_op(0x8400, 0x0400, B, 0); break;
	case i_xor:	alu_op(0x8900, 0x0900, W, 0); break;
	case i_xorb:	alu_op(0x8800, 0x0800, B, 0); break;
	case i_com:	one_dst(0x8D00, W); break;
	case i_comb:	one_dst(0x8C00, B); break;
	case i_test:	one_dst(0x8D04, W); break;
	case i_testb:	one_dst(0x8C04, B); break;
	case i_testl:	testl_op(); break;

/* =================== Bit manipulation =================== */

	case i_bit:	bit_op(0x2700, W); break;
	case i_bitb:	bit_op(0x2600, B); break;
	case i_set:	bit_op(0x2500, W); break;
	case i_setb:	bit_op(0x2400, B); break;
	case i_res:	bit_op(0x2300, W); break;
	case i_resb:	bit_op(0x2200, B); break;
	case i_tset:	one_dst(0x8D06, W); break;
	case i_tsetb:	one_dst(0x8C06, B); break;

/* =================== Shift and rotate =================== */

	case i_sla:	shift_op(0xB309, W, 1); break;
	case i_slab:	shift_op(0xB209, B, 1); break;
	case i_slal:	shift_op(0xB30D, L, 1); break;
	case i_sra:	shift_op(0xB309, W, -1); break;  /* right: negate count */
	case i_srab:	shift_op(0xB209, B, -1); break;
	case i_sral:	shift_op(0xB30D, L, -1); break;
	case i_sll:	shift_op(0xB301, W, 1); break;
	case i_sllb:	shift_op(0xB201, B, 1); break;
	case i_slll:	shift_op(0xB305, L, 1); break;
	case i_srl:	shift_op(0xB301, W, -1); break;  /* right: negate count */
	case i_srlb:	shift_op(0xB201, B, -1); break;
	case i_srll:	shift_op(0xB305, L, -1); break;
	case i_sda:	shift_op(0xB30B, W, 1); break;   /* dynamic: user provides sign */
	case i_sdab:	shift_op(0xB20B, B, 1); break;
	case i_sdal:	shift_op(0xB30F, L, 1); break;
	case i_sdl:	shift_op(0xB303, W, 1); break;
	case i_sdlb:	shift_op(0xB203, B, 1); break;
	case i_sdll:	shift_op(0xB307, L, 1); break;
	case i_rl:	rotate_op(0xB300, W); break;
	case i_rlb:	rotate_op(0xB200, B); break;
	case i_rlc:	rotate_op(0xB308, W); break;
	case i_rlcb:	rotate_op(0xB208, B); break;
	case i_rr:	rotate_op(0xB304, W); break;
	case i_rrb:	rotate_op(0xB204, B); break;
	case i_rrc:	rotate_op(0xB30C, W); break;
	case i_rrcb:	rotate_op(0xB20C, B); break;

/* =================== Multiply and divide =================== */

	case i_mult:	mult_op(0x1900); break;
	case i_multl:	multl_op(0x1800); break;
	case i_div:	div_op(0x1B00); break;
	case i_divl:	divl_op(0x1A00); break;

/* =================== Sign extend =================== */

	case i_exts:	one_reg(0xB10A, L); break;   /* exts rrn: word->long */
	case i_extsb:	one_reg(0xB100, W); break;   /* extsb Rd: sign-extend low byte of word reg */
	case i_extsl:	one_reg(0xB107, L); break;   /* extsl rqn: long->quad */

/* =================== Clear =================== */

	case i_clr:	one_dst(0x8D08, W); break;
	case i_clrb:	one_dst(0x8C08, B); break;

/* =================== Program control =================== */

	case i_call:	call_op(); break;
	case i_calr:	calr_op(); break;
	case i_ret:	ret_op(); break;
	case i_jp:	jp_op(0x1E08); break;	/* unconditional (cc=8, Always True) */
	case i_jr:	jr_op(0xE800); break;	/* unconditional */
	case i_djnz:	djnz_op(0xF000, W); break;
	case i_dbjnz:	djnz_op(0xF000, B); break;

/* =================== Conditional branches (jr cc,addr) =================== */

	/* JR cc encoding: 0xE_cc_disp.  Condition codes from Z8000 manual:
	 * 0=F  1=LT  2=LE  3=ULE  4=OV  5=MI  6=EQ  7=C/ULT
	 * 8=T  9=GE  A=GT  B=UGT  C=NOV D=PL  E=NE  F=NC/UGE
	 */
	case i_jreq:	jr_op(0xE600); break;	/* cc=6  EQ */
	case i_jrne:	jr_op(0xEE00); break;	/* cc=E  NE */
	case i_jrlt:	jr_op(0xE100); break;	/* cc=1  LT */
	case i_jrle:	jr_op(0xE200); break;	/* cc=2  LE */
	case i_jrgt:	jr_op(0xEA00); break;	/* cc=A  GT */
	case i_jrge:	jr_op(0xE900); break;	/* cc=9  GE */
	case i_jrult:	jr_op(0xE700); break;	/* cc=7  ULT/C */
	case i_jrule:	jr_op(0xE300); break;	/* cc=3  ULE */
	case i_jrugt:	jr_op(0xEB00); break;	/* cc=B  UGT */
	case i_jruge:	jr_op(0xEF00); break;	/* cc=F  UGE/NC */
	case i_jrmi:	jr_op(0xE500); break;	/* cc=5  MI */
	case i_jrpl:	jr_op(0xED00); break;	/* cc=D  PL */
	case i_jrov:	jr_op(0xE400); break;	/* cc=4  OV */
	case i_jrnov:	jr_op(0xEC00); break;	/* cc=C  NOV */
	case i_jrc:	jr_op(0xE700); break;	/* cc=7  C */
	case i_jrnc:	jr_op(0xEF00); break;	/* cc=F  NC */

/* =================== Conditional branches (jp cc,addr) =================== */

	/* JP cc encoding: 0x1E_reg_cc + address.  Same condition codes as JR. */
	case i_jpeq:	jp_op(0x1E06); break;	/* cc=6  EQ */
	case i_jpne:	jp_op(0x1E0E); break;	/* cc=E  NE */
	case i_jplt:	jp_op(0x1E01); break;	/* cc=1  LT */
	case i_jple:	jp_op(0x1E02); break;	/* cc=2  LE */
	case i_jpgt:	jp_op(0x1E0A); break;	/* cc=A  GT */
	case i_jpge:	jp_op(0x1E09); break;	/* cc=9  GE */
	case i_jpult:	jp_op(0x1E07); break;	/* cc=7  ULT/C */
	case i_jpule:	jp_op(0x1E03); break;	/* cc=3  ULE */
	case i_jpugt:	jp_op(0x1E0B); break;	/* cc=B  UGT */
	case i_jpuge:	jp_op(0x1E0F); break;	/* cc=F  UGE/NC */
	case i_jpmi:	jp_op(0x1E05); break;	/* cc=5  MI */
	case i_jppl:	jp_op(0x1E0D); break;	/* cc=D  PL */
	case i_jpov:	jp_op(0x1E04); break;	/* cc=4  OV */
	case i_jpnov:	jp_op(0x1E0C); break;	/* cc=C  NOV */
	case i_jpc:	jp_op(0x1E07); break;	/* cc=7  C */
	case i_jpnc:	jp_op(0x1E0F); break;	/* cc=F  NC */

/* =================== Stack operations =================== */

	case i_push:	push_op(W); break;
	case i_pushl:	push_op(L); break;
	case i_pop:	pop_op(W); break;
	case i_popl:	pop_op(L); break;

/* =================== Block transfer =================== */

	case i_ldir:	block_op(0xBB01, 0x00); break;
	case i_ldirb:	block_op(0xBA01, 0x00); break;
	case i_lddr:	block_op(0xBB09, 0x00); break;
	case i_lddrb:	block_op(0xBA09, 0x00); break;
	case i_ldi:	block_op(0xBB01, 0x08); break;
	case i_ldib:	block_op(0xBA01, 0x08); break;
	case i_ldd:	block_op(0xBB09, 0x08); break;
	case i_lddb:	block_op(0xBA09, 0x08); break;

/* =================== Block compare =================== */

	case i_cpir:	cpblk_op(0xBB04); break;
	case i_cpirb:	cpblk_op(0xBA04); break;
	case i_cpdr:	cpblk_op(0xBB0C); break;
	case i_cpdrb:	cpblk_op(0xBA0C); break;
	case i_cpi:	cpblk_op(0xBB00); break;
	case i_cpib:	cpblk_op(0xBA00); break;
	case i_cpd:	cpblk_op(0xBB08); break;
	case i_cpdb:	cpblk_op(0xBA08); break;

/* =================== I/O =================== */

	case i_in:	io_op(0x3D00, 0x3B04, W); break;
	case i_inb:	io_op(0x3C00, 0x3A04, B); break;
	case i_out:	io_op(0x3F00, 0x3B06, W); break;
	case i_outb:	io_op(0x3E00, 0x3A06, B); break;
	case i_ind:
	case i_indb:
	case i_outd:
	case i_outdb:
	case i_indr:
	case i_indrb:
	case i_otdr:
	case i_otdrb:
	case i_ini:
	case i_inib:
	case i_outi:
	case i_outib:
	case i_inir:
	case i_inirb:
	case i_otir:
	case i_otirb:	Prog_Error(E_UNIMPL); break;	/* block I/O not implemented */

/* =================== Pseudo ops =================== */

	case i_long:	if (Dot & 1) Prog_Error(E_ODDADDR);
			ByteWord(L);
			goto pseudo;
	case i_word:	if (Dot & 1) Prog_Error(E_ODDADDR);
			ByteWord(W);
			goto pseudo;
	case i_byte:	ByteWord(B); goto pseudo;
	case i_text:	New_Csect(Text_csect); goto pseudo;
	case i_data:	New_Csect(Data_csect); goto pseudo;
	case i_bss:	New_Csect(Bss_csect); goto pseudo;
	case i_globl:	Globl(); goto pseudo;
	case i_comm:	Comm(); goto pseudo;
	case i_even:	Even(); goto pseudo;
	case i_ascii:	Ascii(0); goto pseudo;
	case i_asciz:	Ascii(1); goto pseudo;
	case i_zerol:	if (Dot & 1) Prog_Error(E_ODDADDR);
			Zerol();
			goto pseudo;
	case i_zerow:	if (Dot & 1) Prog_Error(E_ODDADDR);
			Zerow();
			goto pseudo;

	pseudo:		Code_length = 0;
			break;

	default:	Prog_Error(E_OPCODE);
	};

	if (Code_length) {
	  Put_Words(Code,Code_length);
	  BC = Code_length;
	}
}


/* ========================================================================
 * Instruction encoding routines
 * ======================================================================== */

/* no_op -- no operands */
no_op(opr)
{
	WCode[0] = opr;
	if (numops != 0) Prog_Error(E_NUMOPS);
}

/* di/ei -- takes interrupt mask operand */
di_ei(opr)
{
	WCode[0] = opr;
	if (numops == 1) {
		if (operands[0].type_o != t_immed)
			Prog_Error(E_OPERAND);
		else
			WCode[0] |= (operands[0].value_o & 0x03);
	} else if (numops != 0) Prog_Error(E_NUMOPS);
}

/* sc -- system call, takes immediate operand */
sc_op()
{
	WCode[0] = 0x7F00;
	if (numops != 1) Prog_Error(E_NUMOPS);
	else if (operands[0].type_o != t_immed) Prog_Error(E_OPERAND);
	else WCode[0] |= (operands[0].value_o & 0xFF);
}

/* is control register? (fcw=64, refresh=65, psapseg=66, psapoff=67, nspseg=68, nspoff=69) */
ctrlreg(r) { return(r >= 64 && r <= 69); }

/* control register nibble: fcw=2, refresh=3, psapseg=4, psapoff=5, nspseg=6, nspoff=7 */
ctrl_code(r) { return(r - 62); }

/* ldctl_op -- LDCTL: load control register
 * ldctl Rd, ctrl: 0x7D00 | (Rd << 4) | ctrl_code        (read from ctrl)
 * ldctl ctrl, Rs: 0x7D00 | (Rs << 4) | (ctrl_code + 8)   (write to ctrl)
 */
ldctl_op()
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	/* ldctl Rd, ctrl */
	if (op1->type_o == t_reg && wreg(op1->value_o) &&
	    op2->type_o == t_reg && ctrlreg(op2->value_o)) {
		WCode[0] = 0x7D00 | (regfield(op1->value_o) << 4) | ctrl_code(op2->value_o);
		return;
	}

	/* ldctl ctrl, Rs */
	if (op1->type_o == t_reg && ctrlreg(op1->value_o) &&
	    op2->type_o == t_reg && wreg(op2->value_o)) {
		WCode[0] = 0x7D00 | (regfield(op2->value_o) << 4) | (ctrl_code(op1->value_o) + 8);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* Z8000 manual, LDCTLB: 10001100 Rb 1001 writes FLAGS; ...0001 reads.
 * Only byte registers and FLAGS are accepted; this is unprivileged.
 */
ldctlb_op()
{
	register struct oper *dst, *src;
	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	dst = operands;
	src = &operands[1];
	if (dst->type_o == t_reg && dst->value_o == 70 &&
	    src->type_o == t_reg && breg(src->value_o)) {
		WCode[0] = 0x8C09 | (regfield(src->value_o) << 4);
		return;
	}
	if (src->type_o == t_reg && src->value_o == 70 &&
	    dst->type_o == t_reg && breg(dst->value_o)) {
		WCode[0] = 0x8C01 | (regfield(dst->value_o) << 4);
		return;
	}
	Prog_Error(E_OPERAND);
}

/* one_reg -- single register operand, e.g. neg, com, clr, test, tset, exts, extsb */
one_reg(opr, size)
{
	register struct oper *op = operands;
	int r;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }
	if (op->type_o != t_reg) { Prog_Error(E_OPERAND); return; }
	r = op->value_o;

	if (size == B) {
		if (!breg(r) && !wreg(r)) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (bregfield(r) << 4);
	} else if (size == L) {
		if (!lreg(r)) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (regfield(r) << 4);
	} else {
		if (!wreg(r)) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (r << 4);
	}
}

/* one_dst -- single operand, supports R/IR/DA/X addressing modes
 * Used for neg, com, test, clr, tset, etc.
 * opr is the R-mode opcode (bits 15-14 = 10)
 */
one_dst(opr, size)
{
	register struct oper *op = operands;
	int r, base;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }

	/* R-mode: register operand */
	if (op->type_o == t_reg) {
		one_reg(opr, size);
		return;
	}

	/* base opcode without mode bits (bits 15-14 = 00) */
	base = opr & ~0xC000;

	/* IR mode: @Rn */
	if (op->type_o == t_ireg) {
		if (!wreg(op->reg_o) || op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = base | (regfield(op->reg_o) << 4);
		return;
	}

	/* DA mode: address */
	if (op->type_o == t_normal) {
		WCode[0] = (base | 0x4000);
		rel_val(op, W);
		return;
	}

	/* X mode: offset(Rn) */
	if (op->type_o == t_x) {
		if (!wreg(op->reg_o) || op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = (base | 0x4000) | (regfield(op->reg_o) << 4);
		op->value_o = op->disp_o;
		rel_val(op, W);
		return;
	}

	Prog_Error(E_OPERAND);
}

/* testl -- test long register */
testl_op()
{
	register struct oper *op = operands;
	if (numops != 1) { Prog_Error(E_NUMOPS); return; }
	if (op->type_o != t_reg || !lreg(op->value_o)) { Prog_Error(E_REG); return; }
	WCode[0] = 0x9C08 | (regfield(op->value_o) << 4);
}


/*
 * ld_op -- LD/LDB instruction encoding
 * Handles: reg,reg / reg,@reg / reg,addr / reg,addr(reg) / reg,#imm
 *          @reg,reg / addr,reg / addr(reg),reg / @reg,#imm / addr,#imm
 */
ld_op(size)
{
	register struct oper *op1, *op2;
	int r1, r2, rf1, rf2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	/* reg, src */
	if (op1->type_o == t_reg) {
		r1 = op1->value_o;
		if (size == B && !breg(r1) && !wreg(r1)) { Prog_Error(E_REG); return; }
		if (size == W && !wreg(r1)) { Prog_Error(E_REG); return; }
		rf1 = (size == B) ? bregfield(r1) : regfield(r1);

		/* reg, reg */
		if (op2->type_o == t_reg) {
			r2 = op2->value_o;
			rf2 = (size == B) ? bregfield(r2) : regfield(r2);
			if (size == B)
				WCode[0] = 0xA000 | (rf2 << 4) | rf1;
			else
				WCode[0] = 0xA100 | (rf2 << 4) | rf1;
			return;
		}

		/* reg, @reg */
		if (op2->type_o == t_ireg) {
			if (!wreg(op2->reg_o) || op2->reg_o == 0) { Prog_Error(E_REG); return; }
			if (size == B)
				WCode[0] = 0x2000 | (regfield(op2->reg_o) << 4) | rf1;
			else
				WCode[0] = 0x2100 | (regfield(op2->reg_o) << 4) | rf1;
			return;
		}

		/* reg, addr */
		if (op2->type_o == t_normal) {
			if (size == B)
				WCode[0] = 0x6000 | (0 << 4) | rf1;
			else
				WCode[0] = 0x6100 | (0 << 4) | rf1;
			rel_val(op2, W);
			return;
		}

		/* reg, addr(reg) */
		if (op2->type_o == t_x) {
			if (!wreg(op2->reg_o) || op2->reg_o == 0) { Prog_Error(E_REG); return; }
			if (size == B)
				WCode[0] = 0x6000 | (regfield(op2->reg_o) << 4) | rf1;
			else
				WCode[0] = 0x6100 | (regfield(op2->reg_o) << 4) | rf1;
			op2->value_o = op2->disp_o;
			rel_val(op2, W);
			return;
		}

		/* reg, #imm: LD Rd,#data = 0x21_0_Rd + data (word)
		 *             LDB Rbd,#data = 0x20_0_Rbd + data (byte)
		 */
		if (op2->type_o == t_immed) {
			if (size == W)
				WCode[0] = 0x2100 | rf1;
			else
				WCode[0] = 0x2000 | rf1;
			rel_val(op2, W);
			return;
		}

		Prog_Error(E_OPERAND);
		return;
	}

	/* @reg, src */
	if (op1->type_o == t_ireg) {
		if (!wreg(op1->reg_o) || op1->reg_o == 0) { Prog_Error(E_REG); return; }

		/* @reg, reg */
		if (op2->type_o == t_reg) {
			r2 = op2->value_o;
			rf2 = (size == B) ? bregfield(r2) : regfield(r2);
			if (size == B)
				WCode[0] = 0x2E00 | (regfield(op1->reg_o) << 4) | rf2;
			else
				WCode[0] = 0x2F00 | (regfield(op1->reg_o) << 4) | rf2;
			return;
		}

		/* @reg, #imm */
		if (op2->type_o == t_immed) {
			if (size == B)
				WCode[0] = 0x0C05 | (regfield(op1->reg_o) << 4);
			else
				WCode[0] = 0x0D05 | (regfield(op1->reg_o) << 4);
			rel_val(op2, W);
			return;
		}

		Prog_Error(E_OPERAND);
		return;
	}

	/* addr, reg  or  addr(reg), reg  or  addr/#imm, addr(reg)/#imm */
	if (op1->type_o == t_normal || op1->type_o == t_x) {

		/* addr/addr(reg), reg */
		if (op2->type_o == t_reg) {
			r2 = op2->value_o;
			rf2 = (size == B) ? bregfield(r2) : regfield(r2);

			if (op1->type_o == t_x) {
				if (!wreg(op1->reg_o) || op1->reg_o == 0) { Prog_Error(E_REG); return; }
				if (size == B)
					WCode[0] = 0x6E00 | (regfield(op1->reg_o) << 4) | rf2;
				else
					WCode[0] = 0x6F00 | (regfield(op1->reg_o) << 4) | rf2;
				op1->value_o = op1->disp_o;
			} else {
				if (size == B)
					WCode[0] = 0x6E00 | (0 << 4) | rf2;
				else
					WCode[0] = 0x6F00 | (0 << 4) | rf2;
			}
			rel_val(op1, W);
			return;
		}

		/* addr/addr(reg), #imm: LD address,#data / LD addr(Rd),#data
		 * Word: 0x4D05 / Byte: 0x4C05, identifier nibble = 0x05
		 */
		if (op2->type_o == t_immed) {
			if (op1->type_o == t_x) {
				if (!wreg(op1->reg_o) || op1->reg_o == 0) { Prog_Error(E_REG); return; }
				if (size == B)
					WCode[0] = 0x4C05 | (regfield(op1->reg_o) << 4);
				else
					WCode[0] = 0x4D05 | (regfield(op1->reg_o) << 4);
				op1->value_o = op1->disp_o;
			} else {
				if (size == B)
					WCode[0] = 0x4C05;
				else
					WCode[0] = 0x4D05;
			}
			rel_val(op1, W);
			rel_val(op2, W);
			return;
		}

		Prog_Error(E_OPERAND);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* ldl_op -- LDL instruction (long register loads) */
ldl_op()
{
	register struct oper *op1, *op2;
	int rf1, rf2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	/* rr, src */
	if (op1->type_o == t_reg && lreg(op1->value_o)) {
		rf1 = regfield(op1->value_o);

		/* rr, rr: LDL RRd,RRs = 0x94_Rs_Rd */
		if (op2->type_o == t_reg && lreg(op2->value_o)) {
			rf2 = regfield(op2->value_o);
			WCode[0] = 0x9400 | (rf2 << 4) | rf1;
			return;
		}

		/* rr, @r */
		if (op2->type_o == t_ireg) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = 0x1400 | (regfield(op2->reg_o) << 4) | rf1;
			return;
		}

		/* rr, addr */
		if (op2->type_o == t_normal) {
			WCode[0] = 0x5400 | (0 << 4) | rf1;
			rel_val(op2, W);
			return;
		}

		/* rr, addr(r) */
		if (op2->type_o == t_x) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = 0x5400 | (regfield(op2->reg_o) << 4) | rf1;
			op2->value_o = op2->disp_o;
			rel_val(op2, W);
			return;
		}

		/* rr, #imm */
		if (op2->type_o == t_immed) {
			WCode[0] = 0x1400 | (0 << 4) | rf1;
			rel_val(op2, L);
			return;
		}
	}

	/* @r, rr  or  addr, rr  or  addr(r), rr */
	if (op2->type_o == t_reg && lreg(op2->value_o)) {
		rf2 = regfield(op2->value_o);

		if (op1->type_o == t_ireg) {
			if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = 0x1D00 | (regfield(op1->reg_o) << 4) | rf2;
			return;
		}

		if (op1->type_o == t_normal) {
			WCode[0] = 0x5D00 | (0 << 4) | rf2;
			rel_val(op1, W);
			return;
		}

		if (op1->type_o == t_x) {
			if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = 0x5D00 | (regfield(op1->reg_o) << 4) | rf2;
			op1->value_o = op1->disp_o;
			rel_val(op1, W);
			return;
		}
	}

	Prog_Error(E_OPERAND);
}


/* lda_op -- LDA: load address into register */
lda_op()
{
	register struct oper *op1, *op2;
	int rf1;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (!chk_wreg(op1)) return;
	rf1 = regfield(op1->value_o);

	if (op2->type_o == t_normal) {
		WCode[0] = 0x7600 | (0 << 4) | rf1;
		rel_val(op2, W);
	} else if (op2->type_o == t_x) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = 0x7600 | (regfield(op2->reg_o) << 4) | rf1;
		op2->value_o = op2->disp_o;
		rel_val(op2, W);
	} else Prog_Error(E_OPERAND);
}


/* ldk_op -- LDK: load constant 0-15 into register */
ldk_op()
{
	register struct oper *op1, *op2;
	int rf1;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (!chk_wreg(op1)) return;
	rf1 = regfield(op1->value_o);

	if (op2->type_o != t_immed) { Prog_Error(E_OPERAND); return; }
	if (op2->value_o < 0 || op2->value_o > 15) Prog_Error(E_CONSTANT);
	WCode[0] = 0xBD00 | (rf1 << 4) | (op2->value_o & 0x0F);
}


/* ldm_op -- LDM: load/store multiple registers */
ldm_op()
{
	register struct oper *op1, *op2, *op3;
	int rf;

	if (numops != 3) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];
	op3 = &operands[2];

	/* ldm reg,@reg,#count  or  ldm reg,addr,#count */
	if (op1->type_o == t_reg && wreg(op1->value_o)) {
		rf = regfield(op1->value_o);
		if (op3->type_o != t_immed) { Prog_Error(E_OPERAND); return; }

		if (op2->type_o == t_ireg) {
			WCode[0] = 0x1C01 | (regfield(op2->reg_o) << 4);
			WCode[1] = (rf << 8) | ((op3->value_o - 1) & 0x0F);
			Code_length = 4;
		} else if (op2->type_o == t_normal) {
			WCode[0] = 0x5C01 | (0 << 4);
			WCode[1] = (rf << 8) | ((op3->value_o - 1) & 0x0F);
			Code_length = 4;
			rel_val(op2, W);
		} else Prog_Error(E_OPERAND);
		return;
	}

	/* ldm @reg,reg,#count  or  ldm addr,reg,#count */
	if (op2->type_o == t_reg && wreg(op2->value_o)) {
		rf = regfield(op2->value_o);
		if (op3->type_o != t_immed) { Prog_Error(E_OPERAND); return; }

		if (op1->type_o == t_ireg) {
			WCode[0] = 0x1C09 | (regfield(op1->reg_o) << 4);
			WCode[1] = (rf << 8) | ((op3->value_o - 1) & 0x0F);
			Code_length = 4;
		} else if (op1->type_o == t_normal) {
			WCode[0] = 0x5C09 | (0 << 4);
			WCode[1] = (rf << 8) | ((op3->value_o - 1) & 0x0F);
			Code_length = 4;
			rel_val(op1, W);
		} else Prog_Error(E_OPERAND);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* ldr_op -- LDR/LDRB/LDRL: relative load (PC-relative) */
ldr_op(size)
{
	register struct oper *op1, *op2;
	long offs;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	/* reg,addr */
	if (op1->type_o == t_reg && op2->type_o == t_normal) {
		offs = op2->value_o - (Dot + 4);	/* PC-relative */
		if (size == B)
			WCode[0] = 0x3000 | bregfield(op1->value_o);
		else if (size == W)
			WCode[0] = 0x3100 | regfield(op1->value_o);
		else /* L */
			WCode[0] = 0x3500 | regfield(op1->value_o);
		op2->value_o = offs;
		op2->sym_o = 0;
		rel_val(op2, W);
		return;
	}

	/* addr,reg */
	if (op2->type_o == t_reg && op1->type_o == t_normal) {
		offs = op1->value_o - (Dot + 4);
		if (size == B)
			WCode[0] = 0x3200 | bregfield(op2->value_o);
		else if (size == W)
			WCode[0] = 0x3300 | regfield(op2->value_o);
		else /* L */
			WCode[0] = 0x3700 | regfield(op2->value_o);
		op1->value_o = offs;
		op1->sym_o = 0;
		rel_val(op1, W);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* ex_op -- EX/EXB: exchange register with memory */
ex_op(size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }

	{
		int brf = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);
		if (op2->type_o == t_ireg) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			if (size == B)
				WCode[0] = 0x2C00 | (regfield(op2->reg_o) << 4) | brf;
			else
				WCode[0] = 0x2D00 | (regfield(op2->reg_o) << 4) | brf;
		} else if (op2->type_o == t_normal) {
			if (size == B)
				WCode[0] = 0x6C00 | (0 << 4) | brf;
			else
				WCode[0] = 0x6D00 | (0 << 4) | brf;
			rel_val(op2, W);
		} else if (op2->type_o == t_x) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			if (size == B)
				WCode[0] = 0x6C00 | (regfield(op2->reg_o) << 4) | brf;
			else
				WCode[0] = 0x6D00 | (regfield(op2->reg_o) << 4) | brf;
			op2->value_o = op2->disp_o;
			rel_val(op2, W);
		} else Prog_Error(E_OPERAND);
	}
}


/*
 * alu_op -- ALU instructions: ADD/SUB/CP/AND/OR/XOR (word and byte)
 *
 * Z8000 encoding pattern (bits 15-14 select addressing mode):
 *   R mode:  rr_opr  | (Rs << 4) | Rd          (10_opcode)
 *   IR mode: im_opr  | (Rs << 4) | Rd          (00_opcode, Rs!=0)
 *   IM mode: im_opr  | Rd           + data      (00_opcode, Rs=0)
 *   DA mode: da_opr  | Rd           + address   (01_opcode, Rs=0)
 *   X mode:  da_opr  | (Rs << 4) | Rd + address (01_opcode, Rs!=0)
 *
 * where da_opr = im_opr | 0x4000 and rr_opr = im_opr | 0x8000.
 *
 * mi_id: memory-immediate identifier nibble (0x01 for CP, 0 for ops
 *        that don't support memory-vs-immediate).  Enables:
 *   @Rd,#data:      (size==W ? 0x0D00 : 0x0C00) | (Rd<<4) | mi_id + data
 *   addr,#data:     (size==W ? 0x4D00 : 0x4C00) | mi_id + addr + data
 *   addr(Rd),#data: (size==W ? 0x4D00 : 0x4C00) | (Rd<<4) | mi_id + addr + data
 */
alu_op(rr_opr, im_opr, size, mi_id)
int rr_opr;	/* opcode for register-register (R mode) */
int im_opr;	/* opcode for immediate/indirect (IM/IR mode) */
int mi_id;	/* memory-immediate identifier (0x01 for CP, else 0) */
{
	register struct oper *op1, *op2;
	int rf1, rf2;
	int da_opr;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];
	da_opr = im_opr | 0x4000;	/* DA/X mode = IM/IR base + bit 14 */

	/* reg, src */
	if (op1->type_o == t_reg) {
		rf1 = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);

		/* reg, reg: R mode */
		if (op2->type_o == t_reg) {
			rf2 = (size == B) ? bregfield(op2->value_o) : regfield(op2->value_o);
			WCode[0] = rr_opr | (rf2 << 4) | rf1;
			return;
		}

		/* reg, @reg: IR mode */
		if (op2->type_o == t_ireg) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = im_opr | (regfield(op2->reg_o) << 4) | rf1;
			return;
		}

		/* reg, addr: DA mode */
		if (op2->type_o == t_normal) {
			WCode[0] = da_opr | rf1;
			rel_val(op2, W);
			return;
		}

		/* reg, addr(reg): X mode */
		if (op2->type_o == t_x) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = da_opr | (regfield(op2->reg_o) << 4) | rf1;
			op2->value_o = op2->disp_o;
			rel_val(op2, W);
			return;
		}

		/* reg, #imm: IM mode */
		if (op2->type_o == t_immed) {
			WCode[0] = im_opr | rf1;
			rel_val(op2, W);
			return;
		}

		Prog_Error(E_OPERAND);
		return;
	}

	/* memory, #imm (only supported for CP/CPB via mi_id) */
	if (mi_id && op2->type_o == t_immed) {
		int mi_base;
		mi_base = (size == W) ? 0x0D00 : 0x0C00;

		/* @reg, #imm */
		if (op1->type_o == t_ireg) {
			if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = mi_base | (regfield(op1->reg_o) << 4) | mi_id;
			rel_val(op2, W);
			return;
		}

		/* addr, #imm */
		if (op1->type_o == t_normal) {
			WCode[0] = (mi_base | 0x4000) | mi_id;
			rel_val(op1, W);
			rel_val(op2, W);
			return;
		}

		/* addr(reg), #imm */
		if (op1->type_o == t_x) {
			if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = (mi_base | 0x4000) | (regfield(op1->reg_o) << 4) | mi_id;
			op1->value_o = op1->disp_o;
			rel_val(op1, W);
			rel_val(op2, W);
			return;
		}
	}

	Prog_Error(E_OPERAND);
}


/* alul_op -- ALU long instructions: ADDL/SUBL/CPL
 * opr = R mode opcode (e.g. 0x9600 for ADDL).
 * Derive IM/IR base = opr & ~0x8000, DA/X base = (opr & ~0x8000) | 0x4000.
 */
alul_op(opr)
{
	register struct oper *op1, *op2;
	int rf1, rf2;
	int im_base, da_base;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	im_base = opr & ~0x8000;	/* 0x9600 -> 0x1600 */
	da_base = im_base | 0x4000;	/* 0x1600 -> 0x5600 */

	if (op1->type_o != t_reg || !lreg(op1->value_o)) { Prog_Error(E_REG); return; }
	rf1 = regfield(op1->value_o);

	/* rr, rr: R mode */
	if (op2->type_o == t_reg && lreg(op2->value_o)) {
		rf2 = regfield(op2->value_o);
		WCode[0] = opr | (rf2 << 4) | rf1;
		return;
	}

	/* rr, @r: IR mode */
	if (op2->type_o == t_ireg) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = im_base | (regfield(op2->reg_o) << 4) | rf1;
		return;
	}

	/* rr, #imm: IM mode */
	if (op2->type_o == t_immed) {
		WCode[0] = im_base | rf1;
		rel_val(op2, L);
		return;
	}

	/* rr, addr: DA mode */
	if (op2->type_o == t_normal) {
		WCode[0] = da_base | rf1;
		rel_val(op2, W);
		return;
	}

	/* rr, addr(r): X mode */
	if (op2->type_o == t_x) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = da_base | (regfield(op2->reg_o) << 4) | rf1;
		op2->value_o = op2->disp_o;
		rel_val(op2, W);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* rr_op -- register-register only instructions: ADC/SBC
 * Encoding: opr | (Rs << 4) | Rd  (source in 7-4, dest in 3-0)
 */
rr_op(opr, size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg || op2->type_o != t_reg) {
		Prog_Error(E_OPERAND); return;
	}

	if (size == B)
		WCode[0] = opr | (bregfield(op2->value_o) << 4) | bregfield(op1->value_o);
	else
		WCode[0] = opr | (regfield(op2->value_o) << 4) | regfield(op1->value_o);
}


/* inc_dec -- INC/DEC/INCB/DECB: dst,#n where n=1..16
 * opr = IR mode base (e.g. 0x2900 for INC word).
 * R mode = opr|0x8000, DA/X mode = opr|0x4000.
 */
inc_dec(opr, size)
{
	register struct oper *op1, *op2;
	int rf, n;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op2->type_o != t_immed) { Prog_Error(E_OPERAND); return; }
	n = op2->value_o;
	if (n < 1 || n > 16) Prog_Error(E_CONSTANT);
	n = (n - 1) & 0x0F;	/* encode n-1: 1→0, 2→1, ..., 16→15 */

	/* reg, #n: R mode */
	if (op1->type_o == t_reg) {
		rf = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);
		WCode[0] = (opr | 0x8000) | (rf << 4) | (n & 0x0F);
		return;
	}

	/* @reg, #n: IR mode */
	if (op1->type_o == t_ireg) {
		if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (regfield(op1->reg_o) << 4) | (n & 0x0F);
		return;
	}

	/* addr, #n: DA mode */
	if (op1->type_o == t_normal) {
		WCode[0] = (opr | 0x4000) | (n & 0x0F);
		rel_val(op1, W);
		return;
	}

	/* addr(reg), #n: X mode */
	if (op1->type_o == t_x) {
		if (op1->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = (opr | 0x4000) | (regfield(op1->reg_o) << 4) | (n & 0x0F);
		op1->value_o = op1->disp_o;
		rel_val(op1, W);
		return;
	}

	Prog_Error(E_OPERAND);
}


/* bit_op -- BIT/SET/RES: reg,#n or reg,reg */
bit_op(opr, size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;	/* destination register */
	op2 = &operands[1];	/* bit number */

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }

	if (op2->type_o == t_immed) {
		/* static bit: bit number in imm4 */
		int rf = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);
		if (op2->value_o < 0 || op2->value_o > (size == B ? 7 : 15)) Prog_Error(E_CONSTANT);
		WCode[0] = (opr | 0x8000) | (rf << 4) | (op2->value_o & 0x0F);
	} else if (op2->type_o == t_reg) {
		/* dynamic bit: bit number in word register, target may be byte reg */
		int rf = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);
		WCode[0] = opr | regfield(op2->value_o);
		WCode[1] = rf << 8;
		Code_length = 4;
	} else Prog_Error(E_OPERAND);
}


/* shift_op -- static shift: sla/sra/sll/srl/sda/sdl etc.
 * Form: reg, #count  (count is signed for sda/sdl)
 * sign: 1 = left (positive count), -1 = right (negate count)
 */
shift_op(opr, size, sign)
{
	register struct oper *op1, *op2;
	int rf, count;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }
	rf = (size == B) ? bregfield(op1->value_o) : regfield(op1->value_o);

	if (op2->type_o == t_immed) {
		count = op2->value_o * sign;
		WCode[0] = opr | (rf << 4);
		WCode[1] = count & 0xFFFF;
		Code_length = 4;
	} else if (op2->type_o == t_reg) {
		/* dynamic shift: count in register (always word reg) */
		WCode[0] = opr | (rf << 4);
		WCode[1] = regfield(op2->value_o) << 8;
		Code_length = 4;
	} else Prog_Error(E_OPERAND);
}


/* rotate_op -- RL/RLC/RR/RRC: reg,#1 or reg,#2 */
rotate_op(opr, size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }
	if (op2->type_o != t_immed) { Prog_Error(E_OPERAND); return; }
	if (op2->value_o != 1 && op2->value_o != 2) Prog_Error(E_CONSTANT);

	WCode[0] = opr | (((size == B) ? bregfield(op1->value_o) : regfield(op1->value_o)) << 4);
	if (op2->value_o == 2) WCode[0] |= 0x0002;	/* 2-bit rotate flag */
}


/* mult_op -- MULT rrn,rs (16x16->32 result in pair)
 * opr = IM/IR mode base (0x1900).
 * R mode = opr|0x8000, DA/X mode = opr|0x4000.
 */
mult_op(opr)
{
	register struct oper *op1, *op2;
	int rf1, rf2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg || !lreg(op1->value_o)) { Prog_Error(E_REG); return; }
	rf1 = regfield(op1->value_o);

	/* rr, reg: R mode */
	if (op2->type_o == t_reg && wreg(op2->value_o)) {
		rf2 = regfield(op2->value_o);
		WCode[0] = (opr | 0x8000) | (rf2 << 4) | rf1;
	/* rr, @reg: IR mode */
	} else if (op2->type_o == t_ireg) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (regfield(op2->reg_o) << 4) | rf1;
	/* rr, #imm: IM mode */
	} else if (op2->type_o == t_immed) {
		WCode[0] = opr | rf1;
		rel_val(op2, W);
	/* rr, addr: DA mode */
	} else if (op2->type_o == t_normal) {
		WCode[0] = (opr | 0x4000) | rf1;
		rel_val(op2, W);
	/* rr, addr(reg): X mode */
	} else if (op2->type_o == t_x) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = (opr | 0x4000) | (regfield(op2->reg_o) << 4) | rf1;
		op2->value_o = op2->disp_o;
		rel_val(op2, W);
	} else Prog_Error(E_OPERAND);
}


/* multl_op -- MULTL rqn,rrs (32x32->64)
 * opr = IM/IR base (0x1800). R mode = opr|0x8000.
 * Encoding: (opr|0x8000) | (RRs << 4) | RQd
 */
multl_op(opr)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg || !qreg(op1->value_o)) { Prog_Error(E_REG); return; }
	if (op2->type_o != t_reg || !lreg(op2->value_o)) { Prog_Error(E_REG); return; }

	WCode[0] = (opr | 0x8000) | (regfield(op2->value_o) << 4) | regfield(op1->value_o);
}


/* div_op -- DIV rrn,rs (32/16->16q+16r)
 * opr = IM/IR mode base (0x1B00).
 * R mode = opr|0x8000, DA/X mode = opr|0x4000.
 */
div_op(opr)
{
	register struct oper *op1, *op2;
	int rf1, rf2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg || !lreg(op1->value_o)) { Prog_Error(E_REG); return; }
	rf1 = regfield(op1->value_o);

	/* rr, reg: R mode */
	if (op2->type_o == t_reg && wreg(op2->value_o)) {
		rf2 = regfield(op2->value_o);
		WCode[0] = (opr | 0x8000) | (rf2 << 4) | rf1;
	/* rr, @reg: IR mode */
	} else if (op2->type_o == t_ireg) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = opr | (regfield(op2->reg_o) << 4) | rf1;
	/* rr, #imm: IM mode */
	} else if (op2->type_o == t_immed) {
		WCode[0] = opr | rf1;
		rel_val(op2, W);
	/* rr, addr: DA mode */
	} else if (op2->type_o == t_normal) {
		WCode[0] = (opr | 0x4000) | rf1;
		rel_val(op2, W);
	/* rr, addr(reg): X mode */
	} else if (op2->type_o == t_x) {
		if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = (opr | 0x4000) | (regfield(op2->reg_o) << 4) | rf1;
		op2->value_o = op2->disp_o;
		rel_val(op2, W);
	} else Prog_Error(E_OPERAND);
}


/* divl_op -- DIVL rqn,rrs (64/32->32q+32r) */
divl_op(opr)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg || !qreg(op1->value_o)) { Prog_Error(E_REG); return; }
	if (op2->type_o != t_reg || !lreg(op2->value_o)) { Prog_Error(E_REG); return; }

	WCode[0] = (opr | 0x8000) | (regfield(op2->value_o) << 4) | regfield(op1->value_o);
}


/* call_op -- CALL: call @reg, call addr, call addr(reg) */
call_op()
{
	register struct oper *op = operands;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }

	if (op->type_o == t_ireg) {
		if (op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = 0x1F00 | (regfield(op->reg_o) << 4);
	} else if (op->type_o == t_normal) {
		WCode[0] = 0x5F00 | (0 << 4);
		rel_val(op, W);
	} else if (op->type_o == t_x) {
		if (op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = 0x5F00 | (regfield(op->reg_o) << 4);
		op->value_o = op->disp_o;
		rel_val(op, W);
	} else Prog_Error(E_OPERAND);
}


/* calr_op -- CALR: call relative (PC + displacement) */
calr_op()
{
	long offs;
	register struct oper *op = operands;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }
	if (op->type_o == t_reg) { Prog_Error(E_OPERAND); return; }

	/* Try as SDI on pass 1 */
	if (Pass == 1) {
		if (op->type_o == t_normal && !(op->flags_o & O_COMPLEX)) {
			Code_length = makesdi(op, 4, Dot + 2,
				sdi_bound(2, -4096L, 4094L,
				 sdi_bound(4, -32768L, 32767L, (struct blist *)0)));
			return;
		}
	}

	if (op->sym_o == 0 || op->sym_o->csect_s != Cur_csect) {
		/* can't do relative -- use CALL with absolute address */
		WCode[0] = 0x5F00;
		rel_val(op, W);
		return;
	}

	offs = op->value_o - (Dot + 2);
	if (offs < -4096 || offs > 4094 || (offs & 1)) {
		/* out of range for CALR, use CALL */
		WCode[0] = 0x5F00;
		rel_val(op, W);
		return;
	}

	WCode[0] = 0xD000 | ((offs >> 1) & 0x0FFF);
}


/* ret_op -- RET cc */
ret_op()
{
	if (numops == 0) {
		WCode[0] = 0x9E08;	/* unconditional return */
	} else if (numops == 1) {
		/* conditional return -- cc already encoded in opindex for now, use unconditional */
		WCode[0] = 0x9E08;
		Prog_Error(E_UNIMPL);	/* conditional ret not yet supported */
	} else Prog_Error(E_NUMOPS);
}


/* jp_op -- JP: jump to address (conditional or unconditional) */
jp_op(opr)
{
	register struct oper *op = operands;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }

	if (op->type_o == t_ireg) {
		if (op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = (opr & 0xFF0F) | (regfield(op->reg_o) << 4);
	} else if (op->type_o == t_normal) {
		WCode[0] = ((opr | 0x4000) & 0xFF0F) | (0 << 4);
		rel_val(op, W);
	} else if (op->type_o == t_x) {
		if (op->reg_o == 0) { Prog_Error(E_REG); return; }
		WCode[0] = ((opr | 0x4000) & 0xFF0F) | (regfield(op->reg_o) << 4);
		op->value_o = op->disp_o;
		rel_val(op, W);
	} else Prog_Error(E_OPERAND);
}


/* jr_op -- JR: jump relative (PC + displacement) */
jr_op(opr)
{
	long offs;
	register struct oper *op = operands;

	if (numops != 1) { Prog_Error(E_NUMOPS); return; }
	if (op->type_o == t_reg) { Prog_Error(E_OPERAND); return; }

	/* SDI on pass 1 */
	if (Pass == 1 && op->type_o == t_normal && !(op->flags_o & O_COMPLEX)) {
		Code_length = makesdi(op, 4, Dot + 2,
			sdi_bound(2, -256L, 254L,
			 sdi_bound(4, -32768L, 32767L, (struct blist *)0)));
		return;
	}

	if (op->sym_o == 0 || op->sym_o->csect_s != Cur_csect) {
		/* use JP with absolute address */
		jp_abs(opr, op);
		return;
	}

	offs = op->value_o - (Dot + 2);
	if (offs < -256 || offs > 254 || (offs & 1)) {
		/* use JP with absolute address */
		jp_abs(opr, op);
		return;
	}

	WCode[0] = opr | ((offs >> 1) & 0xFF);
}


/* jp_abs -- convert a jr that's out of range to a jp */
jp_abs(jr_opr, op)
  struct oper *op;
{
	int cc;

	/* extract condition code from jr opcode: bits 11-8 */
	cc = (jr_opr >> 8) & 0x0F;
	WCode[0] = 0x5E00 | cc;
	rel_val(op, W);
}


/* djnz_op -- DJNZ/DBJNZ: decrement and jump if not zero */
djnz_op(opr, size)
{
	register struct oper *op1, *op2;
	long offs;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }

	if (op2->sym_o == 0 || op2->sym_o->csect_s != Cur_csect) {
		Prog_Error(E_RELOCATE);
		return;
	}

	offs = op2->value_o - (Dot + 2);
	if (offs > 0 || offs < -254 || (offs & 1)) Prog_Error(E_OFFSET);

	WCode[0] = 0xF000
		 | (((size == B) ? bregfield(op1->value_o) : regfield(op1->value_o)) << 8)
		 | (size == B ? 0 : 0x80)
		 | (((-offs) >> 1) & 0x7F);
}


/* push_op -- PUSH/PUSHL @rd,rs or @rd,#imm or @rd,addr */
push_op(size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op1->type_o != t_ireg) { Prog_Error(E_OPERAND); return; }
	if (op1->reg_o == 0) { Prog_Error(E_REG); return; }

	if (size == W) {
		if (op2->type_o == t_reg && wreg(op2->value_o)) {
			WCode[0] = 0x9300 | (regfield(op1->reg_o) << 4) | regfield(op2->value_o);
		} else if (op2->type_o == t_immed) {
			/* PUSH @Rd,#data: 0x0D_Rd_9 + data */
			WCode[0] = 0x0D09 | (regfield(op1->reg_o) << 4);
			rel_val(op2, W);
		} else if (op2->type_o == t_ireg) {
			WCode[0] = 0x1300 | (regfield(op1->reg_o) << 4) | regfield(op2->reg_o);
		} else if (op2->type_o == t_normal) {
			WCode[0] = 0x5300 | (regfield(op1->reg_o) << 4);
			rel_val(op2, W);
		} else if (op2->type_o == t_x) {
			if (op2->reg_o == 0) { Prog_Error(E_REG); return; }
			WCode[0] = 0x5300 | (regfield(op1->reg_o) << 4) | regfield(op2->reg_o);
			op2->value_o = op2->disp_o;
			rel_val(op2, W);
		} else Prog_Error(E_OPERAND);
	} else { /* L */
		if (op2->type_o == t_reg && lreg(op2->value_o)) {
			WCode[0] = 0x9100 | (regfield(op1->reg_o) << 4) | regfield(op2->value_o);
		} else if (op2->type_o == t_ireg) {
			WCode[0] = 0x1100 | (regfield(op1->reg_o) << 4) | regfield(op2->value_o);
		} else Prog_Error(E_OPERAND);
	}
}


/* pop_op -- POP/POPL rd,@rs */
pop_op(size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];

	if (op2->type_o != t_ireg) { Prog_Error(E_OPERAND); return; }
	if (op2->reg_o == 0) { Prog_Error(E_REG); return; }

	if (size == W) {
		if (op1->type_o == t_reg && wreg(op1->value_o)) {
			WCode[0] = 0x9700 | (regfield(op2->reg_o) << 4) | regfield(op1->value_o);
		} else if (op1->type_o == t_ireg) {
			WCode[0] = 0x1700 | (regfield(op2->reg_o) << 4) | regfield(op1->value_o);
		} else if (op1->type_o == t_normal) {
			WCode[0] = 0x5700 | (regfield(op2->reg_o) << 4);
			rel_val(op1, W);
		} else Prog_Error(E_OPERAND);
	} else { /* L */
		if (op1->type_o == t_reg && lreg(op1->value_o)) {
			WCode[0] = 0x9500 | (regfield(op2->reg_o) << 4) | regfield(op1->value_o);
		} else if (op1->type_o == t_ireg) {
			WCode[0] = 0x1500 | (regfield(op2->reg_o) << 4) | regfield(op1->value_o);
		} else Prog_Error(E_OPERAND);
	}
}


/* block_op -- LDIR/LDDR/LDI/LDD: @rd,@rs,rn
 * w2_flags: low nibble OR'd into word 2 (0x00 for repeat, 0x08 for non-repeat)
 */
block_op(opr, w2_flags)
{
	register struct oper *op1, *op2, *op3;

	if (numops != 3) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];
	op3 = &operands[2];

	if (op1->type_o != t_ireg || op2->type_o != t_ireg ||
	    op3->type_o != t_reg || !wreg(op3->value_o)) {
		Prog_Error(E_OPERAND);
		return;
	}
	if (op1->reg_o == 0 || op2->reg_o == 0) { Prog_Error(E_REG); return; }

	WCode[0] = opr | (regfield(op2->reg_o) << 4);
	WCode[1] = (regfield(op3->value_o) << 8) | (regfield(op1->reg_o) << 4) | w2_flags;
	Code_length = 4;
}


/* cpblk_op -- CPIR/CPDR/CPI/CPD: rd,@rs,rn */
cpblk_op(opr)
{
	register struct oper *op1, *op2, *op3;

	if (numops != 3) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;
	op2 = &operands[1];
	op3 = &operands[2];

	if (op1->type_o != t_reg || !wreg(op1->value_o) ||
	    op2->type_o != t_ireg || op3->type_o != t_reg || !wreg(op3->value_o)) {
		Prog_Error(E_OPERAND);
		return;
	}
	if (op2->reg_o == 0) { Prog_Error(E_REG); return; }

	WCode[0] = opr | (regfield(op2->reg_o) << 4);
	WCode[1] = (regfield(op3->value_o) << 8) | (regfield(op1->value_o) << 4) | 0x08;
	Code_length = 4;
}


/* io_op -- IN/INB/OUT/OUTB: data_reg, @port_reg  or  data_reg, #port
 * r_opr:  R mode opcode (e.g. 0x3D00 for IN)
 * da_opr: DA mode opcode (e.g. 0x3B04 for IN)
 * R mode:  WCode[0] = r_opr  | (port_reg << 4) | data_reg
 * DA mode: WCode[0] = da_opr | (data_reg << 4); + port address word
 */
io_op(r_opr, da_opr, size)
{
	register struct oper *op1, *op2;

	if (numops != 2) { Prog_Error(E_NUMOPS); return; }
	op1 = operands;	/* data register */
	op2 = &operands[1];	/* port: @reg or #imm */

	if (op1->type_o != t_reg) { Prog_Error(E_OPERAND); return; }

	if (op2->type_o == t_ireg) {
		/* R mode: port in indirect register */
		WCode[0] = r_opr | (regfield(op2->reg_o) << 4)
			| ((size == B) ? bregfield(op1->value_o) : regfield(op1->value_o));
	} else if (op2->type_o == t_immed) {
		/* DA mode: port is immediate address */
		WCode[0] = da_opr
			| (((size == B) ? bregfield(op1->value_o) : regfield(op1->value_o)) << 4);
		rel_val(op2, W);
	} else Prog_Error(E_OPERAND);
}
