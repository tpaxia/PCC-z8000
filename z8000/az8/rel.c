#include "mical.h"
#include "../b.out.h"

/*  Handle output file processing for a.out files
 *  Adapted from 68000 version for Z8000.
 *  Z8000 is big-endian like 68000, same byte order in object files.
 *  Main difference: pointers are 16-bit (RWORD) not 32-bit.
 *
 *  a.out layout: header(16) | text | data | text_reloc | data_reloc | symbols
 */

FILE *tout;		/* text portion of output file */
FILE *dout;		/* data portion of output file */
FILE *rtout;		/* text relocation commands */
FILE *rdout;		/* data relocation commands */

long rtsize;		/* size of text relocation area */
long rdsize;		/* size of data relocation area */

char rname[STR_MAX];	/* name of file for relocation commands */
char rdname[STR_MAX];	/* name of file for data relocation commands */

struct bhdr filhdr;	/* header for b.out files, contains sizes */

/* Initialize files for output and write out the header.
 * Symbols are deferred to Fix_Rel() (written after relocation in a.out order).
 */

Rel_Header()
{
	if ((tout = fopen(Rel_name, "w")) == NULL ||
		(dout = fopen(Rel_name, "r+")) == NULL)
		Sys_Error("open on output file %s failed", Rel_name);

	/* Text and data relocation commands go to separate files. They used to
	 * share one, with the data commands placed after the space the first
	 * pass estimated for text; when the final pass emitted more text
	 * commands than estimated, the two ran into each other. */
	Concat(rname, Source_name, ".tmpr");
	Concat(rdname, Source_name, ".tmpd");
	if ((rtout = fopen(rname, "w")) == NULL)
		Sys_Error("open on output file %s failed", rname);
	if ((rdout = fopen(rdname, "w")) == NULL)
		Sys_Error("open on output file %s failed", rdname);
	filhdr.fmagic = FMAGIC;
	filhdr.tsize = tsize;
	filhdr.dsize = dsize;
	filhdr.bsize = bsize;
	filhdr.ssize = 0;
	filhdr.entry = 0;
	filhdr.trsize = rtsize;
	filhdr.drsize = rdsize;

	fseek(tout, 0L, 0);
	put68(tout, &filhdr.fmagic, 2);
	put68(tout, &filhdr.tsize, 2);
	put68(tout, &filhdr.dsize, 2);
	put68(tout, &filhdr.bsize, 2);
	put68(tout, &filhdr.ssize, 2);
	put68(tout, &filhdr.entry, 2);
	put68(tout, &filhdr.trsize, 2);
	put68(tout, &filhdr.drsize, 2);

	fseek(tout, (long)(TEXTPOS), 0);	/* seek to start of text */
	fseek(dout, (long)(DATAPOS), 0);
	rtsize = 0;
	rdsize = 0;
}

/*
 * Fix_Rel -	Fix up the object file.
 *		Write relocation, then symbols, then re-write header.
 */
Fix_Rel()
{
	long i;
	long Sym_Write();
	register FILE *fin, *fout;

	filhdr.trsize = rtsize;
	filhdr.drsize = rdsize;
	fclose(rtout);
	fclose(rdout);
	if ((fin = fopen(rname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rname);

	fout = tout;

	/* first write text relocation commands */
	fseek(fout, (long)(RTEXTPOS), 0);
	for (i=0; i<rtsize; i++)
		putc(getc(fin), fout);

	/* then the data segment relocation commands */
	fclose(fin);
	unlink(rname);
	if ((fin = fopen(rdname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rdname);
	for (i=0; i<rdsize; i++)
		putc(getc(fin), fout);

	/* write symbols after relocation (a.out order) */
	filhdr.ssize = Sym_Write(fout);

	/* now re-write header */
	fseek(fout, 0, 0);
	put68(tout, &filhdr.fmagic, 2);
	put68(tout, &filhdr.tsize, 2);
	put68(tout, &filhdr.dsize, 2);
	put68(tout, &filhdr.bsize, 2);
	put68(tout, &filhdr.ssize, 2);
	put68(tout, &filhdr.entry, 2);
	put68(tout, &filhdr.trsize, 2);
	put68(tout, &filhdr.drsize, 2);
	fclose(fin);
	unlink(rdname);
}

/* rel_val -	Puts value of operand into next bytes of Code
 * updating Code_length. Put_Rel is called to handle possible relocation.
 * If size=L a longword is stored, otherwise a word is stored
 */
rel_val(opnd,size)
register struct oper *opnd;
{	register int i;
	register struct sym_bkt *sp;
	long val;
	char *CCode;

	i = Code_length>>1;	/* get index into WCode */
	if (sp = opnd->sym_o)
		Put_Rel(opnd, size, Dot + Code_length);
	val = opnd->value_o;
	switch(size)
	{
	case L:
		WCode[i++] = val>>16;
		Code_length += 2;
	case W:
		WCode[i] = val;
		Code_length += 2;
		break;
	case B:
		CCode = (char *)WCode;
		CCode[Code_length++] = val;
	}
 }

/* Put_Words -- puts whole words, enforcing the mapping of bytes to words.
 * Z8000 is big-endian like 68000.
 */

#ifdef z8000
Put_Words(code,nbytes)
  char *code;
  {	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d",nbytes);
	Put_Text(code,nbytes);
}
#endif

#ifndef z8000
Put_Words(code,nbytes)
register char *code;
{	register char *cc, ch;
	register int i;
	char tcode[100];

	cc = tcode;
	for (i=0; i<nbytes; i++) tcode[i] = code[i];
	i = nbytes>>1;
	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d\n",nbytes);
	while (i--) { ch = *cc; *cc = cc[1]; *++cc = ch; cc++; }
	Put_Text(tcode,nbytes);
}
#endif

/* Put_Text -	Write out text to proper portion of file */

Put_Text(code,length)
 register char *code;
 {	if (Pass != 2) return;
	if (Cur_csect == Text_csect) fwrite(code, length, 1, tout);
	else if (Cur_csect == Data_csect) fwrite(code, length, 1, dout);
	else return;	/* ignore if bss segment */
 }

/* Pad_Text -	Write zero bytes to pad out proper portion of file */

Pad_Text(length)
 register length;
 {
	register FILE *f;
	if (Pass != 2) return;
	if (Cur_csect == Text_csect) f = tout;
	else if (Cur_csect == Data_csect) f = dout;
	else return;	/* ignore if bss segment */
	while (length-- > 0)
		putc(0, f);
 }

/* set up relocation word for operand:
 *  opnd	pointer to operand structure
 *  size	0 = byte, 1 = word, 2 = long/address
 *  offset	offset into WCode & WReloc array
 *
 * Note: Z8000 pointers are 16-bit (word), so most relocations
 * will be RWORD rather than RLONG.
 */

Put_Rel(opnd,size,offset)
struct oper *opnd;
int size;
long offset;
{
	struct reloc r;
	if (opnd->sym_o == 0) return;	/* no relocation */
	if (Cur_csect == Text_csect)
		rtsize += rel_cmd(&r, opnd, size, offset, rtout);
	else if (Cur_csect == Data_csect)
		rdsize += rel_cmd(&r, opnd, size, offset - tsize, rdout);
	else return;	/* just ignore if bss segment */
}


/* rel_cmd -	Generate a relocation command and output */

static	int	sizes[] = {  RBYTE, RWORD, RLONG  };

rel_cmd(rp, opnd, size, offset, file)
register struct reloc *rp;
struct oper *opnd;
int size;
long offset;
FILE *file;
{
	int csid;
	register struct csect *csp;
	register struct sym_bkt *sp;

	sp = opnd->sym_o;
	csp = sp->csect_s;
	if (Pass == 2) {
		rp->rsymbol = 0;
		rp->rinfo = 0;
		if (!(sp->attr_s & S_DEF)
		 && (sp->attr_s & S_EXT)) {
			rp->rinfo = (rp->rinfo & ~RSEGMNT) | REXT;
			rp->rsymbol = sp->id_s;
		}
		else if (csp == Text_csect)
			rp->rinfo = (rp->rinfo & ~RSEGMNT) | RTEXT;
		else if (csp == Data_csect)
			rp->rinfo = (rp->rinfo & ~RSEGMNT) | RDATA;
		else if (csp == Bss_csect)
			rp->rinfo = (rp->rinfo & ~RSEGMNT) | RBSS;
		else Prog_Error(E_RELOCATE);
		rp->rpos = offset;
		rp->rinfo = (rp->rinfo & ~RSIZE) | sizes[size];
		rp->rinfo = (rp->rinfo & ~RDISP);
		put68(file, &rp->rinfo, sizeof(rp->rinfo));
		put68(file, &rp->rsymbol, sizeof(rp->rsymbol));
		put68(file, &rp->rpos, sizeof(rp->rpos));
	}
	return(RELOC_DISKSIZE);
}
