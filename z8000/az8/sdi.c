#include "mical.h"

static char *sdi = "~|^`s.sdi.c R1.7 on 6/27/80";

/*
 * module to handle span-dependent instructions, e.g. jbr
 * see CACM Vol 21 No 4 April 1978 for a description of the
 * algorithm for resolving sdi's between the 1st and 2nd pass
 */


struct sdi_form {
	struct sdi_form *next;
	struct csect *csect;
	struct blist *bounds;
	int gleng;
	long baseoff;	/* PC-relative base minus instruction location */
};
static struct sdi_form *forms;

struct sdi {	/* information for span dependent instructions (sdi's) */
	struct sdi *sdi_next;           /* next sdi in list */
	struct sdi_form *form;	/* shared section and branch forms */
	long int sdi_loc;		/* location of the sdi */
	struct sym_bkt *sdi_sym;	/* symbol part of the sdi address */
	int sdi_leng;			/* actual length of the sdi */
} *sdi_list = 0;	/* linked list of sdi's descriptors */

#define SDI_INCR 32
struct sdi_block {
	struct sdi_block *next;
	struct sdi records[SDI_INCR];
};
static struct sdi_block *blocks;
static int used = SDI_INCR;

struct blist {	/* length and bounds information for various forms of sdi's */
	struct blist *b_next;	/* next element in list */
	struct blist *b_pool;	/* all immutable, shared bounds nodes */
	int b_length;		/* length of this form of the sdi */
	long int b_lbound;	/* lower and upper bound on the span */
	long int b_ubound;	/* for this form of the sdi */
};
static struct blist *bound_pool;

/*
 * routine to create a sdi descriptor and insert it into the list
 */
makesdi(op, gleng, base, bounds)
struct oper *op;	/* the operand of the sdi */
int gleng;		/* the length of the most general form of the sdi */
long int base;		/* origin of the the initial span of the sdi, to be */
			/* corrected between pass1 and pass2 by subtracting */
			/* from the value of the symbol of the operand */
			/* i.e. 0 for resolving short absolute address modes */
			/* Dot[+increment] for PC relative addresses */
struct blist *bounds;	/* list of lengths & bounds for the sdi */
{
	register struct sdi *s, **p;
	struct sdi_form *f;
	struct sdi_block *block;
	extern struct csect *Cur_csect;

	if (op->flags_o&O_COMPLEX)	/* not a simple address */
		return;
	for (f = forms; f; f = f->next)
		if (f->csect == Cur_csect && f->bounds == bounds &&
		    f->gleng == gleng && f->baseoff == base-Dot) break;
	if (!f) {
		if ((f = (struct sdi_form *)calloc(1,sizeof *f)) == NULL)
			Sys_Error("sdi form storage exceeded\n");
		f->csect = Cur_csect;
		f->bounds = bounds;
		f->gleng = gleng;
		f->baseoff = base-Dot;
		f->next = forms;
		forms = f;
	}
	/* Avoid a malloc header and alignment overhead for every branch. */
	if (used == SDI_INCR) {
		block = (struct sdi_block *)calloc(1,sizeof *block);
		if (!block) Sys_Error("sdi storage exceeded\n");
		block->next = blocks;
		blocks = block;
		used = 0;
	}
	s = &blocks->records[used++];
	s->sdi_loc = Dot;
	s->form = f;
	s->sdi_sym = op->sym_o;
	s->sdi_leng = bounds->b_length;	/* shortest length */
	for (p = &sdi_list; *p; p = &(*p)->sdi_next)
		if (s->sdi_loc < (*p)->sdi_loc)
			break;
	s->sdi_next = *p;
	*p = s;
	return(s->sdi_leng);	/* return the current length */
}

/*
 * Intern an immutable bounds list, sorted by increasing b_length.
 * Hundreds of branches use the same two forms. Allocating two private
 * bounds nodes for each branch exhausts a native 64K data space.
 */
struct blist *sdi_bound(leng, lbound, ubound, next)
int leng;		/* length of this form of the sdi */
long int lbound;	/* lower bound of span */
long int ubound;	/* upper bound */
struct blist *next;	/* target blist */
{
	register struct blist *b;

	if (next && leng > next->b_length)
		return sdi_bound(next->b_length, next->b_lbound,
			next->b_ubound, sdi_bound(leng, lbound, ubound, next->b_next));
	for (b = bound_pool; b; b = b->b_pool)
		if (b->b_length == leng && b->b_lbound == lbound &&
		    b->b_ubound == ubound && b->b_next == next)
			return b;

	if ((b = (struct blist *)calloc(1,sizeof *b)) == NULL)
		Sys_Error("sdi bound list storage exceeded\n");
	b->b_length = leng;
	b->b_lbound = lbound;
	b->b_ubound = ubound;
	b->b_next = next;
	b->b_pool = bound_pool;
	bound_pool = b;
	return b;
}

/*
 * resolve sdi's between pass1 and pass2
 * basic algorithm is to repeatedly look for sdi that must use the
 * long form, and update the span of other sdi's.
 * When this terminates, all remaining sdi's can use the short form
 */
sdi_resolve()
{
	register struct sdi *s;
	register int t;
	int changed;

	do {
		changed = 0;
		for (s = sdi_list; s; s = s->sdi_next) {
			if ((t = sdi_len(s) - s->sdi_leng) > 0) {
				s->sdi_leng += t;
				changed = 1;
			} else if (t < 0)
				Sys_Error("Pathological sdi\n");
		}
	} while (changed);
}

/*
 * compute the length of the specified sdi by searching the bounds list
 */
sdi_len(s)
register struct sdi *s;
{
	register struct blist *b;
	long span;
	if (!(s->sdi_sym->attr_s & S_DEF) || s->form->csect != s->sdi_sym->csect_s)
		return(s->form->gleng);
	/* Derive both addresses from original locations and relaxed branches.
	 * No per-branch copy of the adjusted PC or span is needed.
	 */
	span = s->sdi_sym->value_s +
		sdi_inc(s->form->csect, s->sdi_sym->value_s) -
		(s->sdi_loc + s->form->baseoff +
		 sdi_inc(s->form->csect, s->sdi_loc));
	for (b = s->form->bounds; b; b = b->b_next)
		if (b->b_lbound <= span && span <= b->b_ubound)
			return(b->b_length);
	return(s->form->gleng);
}

/*
 * return the total number of extra bytes due to long sdi's before
 * the specified offset in the specified csect
 */
sdi_inc(csect, offset)
register struct csect *csect;
long int offset;
{
	register struct sdi *s;
	register int total;

	for (s = sdi_list, total = 0; s; s = s->sdi_next) {
		if (csect == s->form->csect) {
			if (offset <= s->sdi_loc)
				break;
			total += s->sdi_leng - s->form->bounds->b_length;
		}
	}
	return(total);
}

/*
 * release all sdi descriptors
 */
sdi_free()
{
	struct sdi_block *block, *bn;
	struct sdi_form *f, *fn;
	struct blist *b, *next;
	for (block = blocks; block; block = bn) {
		bn = block->next;
		free(block);
	}
	blocks = (struct sdi_block *)0;
	used = SDI_INCR;
	sdi_list = (struct sdi *)0;
	for (f = forms; f; f = fn) {
		fn = f->next;
		free(f);
	}
	forms = (struct sdi_form *)0;
	for (b = bound_pool; b; b = next) {
		next = b->b_pool;
		free(b);
	}
	bound_pool = (struct blist *)0;
}
