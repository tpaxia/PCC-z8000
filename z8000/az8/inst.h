/* dispatch numbers for Z8000 instructions.  These numbers are placed in
 * the code_i component of each instruction bucket by init.c and are used
 * by ins.c for dispatching.
 */

/* load and move instructions */
#define i_ld	1
#define i_ldb	2
#define i_ldl	3
#define i_lda	4
#define i_ldk	5
#define i_ldm	6
#define i_ldr	7
#define i_ldrb	8
#define i_ldrl	9
#define i_ex	10
#define i_exb	11

/* arithmetic instructions */
#define i_add	12
#define i_addb	13
#define i_addl	14
#define i_adc	15
#define i_adcb	16
#define i_sub	17
#define i_subb	18
#define i_subl	19
#define i_sbc	20
#define i_sbcb	21
#define i_inc	22
#define i_incb	23
#define i_dec	24
#define i_decb	25
#define i_neg	26
#define i_negb	27
#define i_da	28
#define i_dab	29

/* compare instructions */
#define i_cp	30
#define i_cpb	31
#define i_cpl	32

/* logical instructions */
#define i_and	33
#define i_andb	34
#define i_or	35
#define i_orb	36
#define i_xor	37
#define i_xorb	38
#define i_com	39
#define i_comb	40
#define i_test	41
#define i_testb	42
#define i_testl	43

/* bit manipulation */
#define i_bit	44
#define i_bitb	45
#define i_set	46
#define i_setb	47
#define i_res	48
#define i_resb	49
#define i_tset	50
#define i_tsetb	51

/* shift and rotate */
#define i_sla	52
#define i_slab	53
#define i_slal	54
#define i_sra	55
#define i_srab	56
#define i_sral	57
#define i_sll	58
#define i_sllb	59
#define i_slll	60
#define i_srl	61
#define i_srlb	62
#define i_srll	63
#define i_sda	64
#define i_sdab	65
#define i_sdal	66
#define i_sdl	67
#define i_sdlb	68
#define i_sdll	69
#define i_rl	70
#define i_rlb	71
#define i_rlc	72
#define i_rlcb	73
#define i_rr	74
#define i_rrb	75
#define i_rrc	76
#define i_rrcb	77

/* multiply and divide */
#define i_mult	78
#define i_multl	79
#define i_div	80
#define i_divl	81

/* sign extend */
#define i_exts	82
#define i_extsb	83
#define i_extsl	84

/* clear */
#define i_clr	85
#define i_clrb	86

/* program control */
#define i_call	87
#define i_calr	88
#define i_ret	89
#define i_jp	90
#define i_jr	91
#define i_djnz	92
#define i_dbjnz	93

/* conditional jp variants */
#define i_jreq	94
#define i_jrne	95
#define i_jrlt	96
#define i_jrle	97
#define i_jrgt	98
#define i_jrge	99
#define i_jrult	100
#define i_jrule	101
#define i_jrugt	102
#define i_jruge	103
#define i_jrmi	104
#define i_jrpl	105
#define i_jrov	106
#define i_jrnov	107
#define i_jrc	108
#define i_jrnc	109

#define i_jpeq	110
#define i_jpne	111
#define i_jplt	112
#define i_jple	113
#define i_jpgt	114
#define i_jpge	115
#define i_jpult	116
#define i_jpule	117
#define i_jpugt	118
#define i_jpuge	119
#define i_jpmi	120
#define i_jppl	121
#define i_jpov	122
#define i_jpnov	123
#define i_jpc	124
#define i_jpnc	125

/* stack operations */
#define i_push	126
#define i_pushl	127
#define i_pop	128
#define i_popl	129

/* block transfer and search */
#define i_ldir	130
#define i_ldirb	131
#define i_lddr	132
#define i_lddrb	133
#define i_ldi	134
#define i_ldib	135
#define i_ldd	136
#define i_lddb	137
#define i_cpir	138
#define i_cpirb	139
#define i_cpdr	140
#define i_cpdrb	141
#define i_cpi	142
#define i_cpib	143
#define i_cpd	144
#define i_cpdb	145

/* I/O instructions */
#define i_in	146
#define i_inb	147
#define i_out	148
#define i_outb	149
#define i_ind	150
#define i_indb	151
#define i_outd	152
#define i_outdb	153
#define i_indr	154
#define i_indrb	155
#define i_otdr	156
#define i_otdrb	157
#define i_ini	158
#define i_inib	159
#define i_outi	160
#define i_outib	161
#define i_inir	162
#define i_inirb	163
#define i_otir	164
#define i_otirb	165

/* special instructions */
#define i_nop	166
#define i_halt	167
#define i_di	168
#define i_ei	169
#define i_sc	170
#define i_iret	171
#define i_mbit	172
#define i_mreq	173
#define i_mres	174
#define i_mset	175
#define i_ldctl	176
#define i_ldctlb 177

/* pseudo ops */
#define i_long	200
#define i_word	201
#define i_byte	202
#define i_text	203
#define i_data	204
#define i_bss	205
#define i_globl 206
#define i_comm	207
#define i_even	208
#define i_ascii 209
#define i_asciz 210
#define i_zerol 211
#define i_zerow 212
