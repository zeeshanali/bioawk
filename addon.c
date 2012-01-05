#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "awk.h"

int bio_flag = 0, bio_fmt = BIO_NULL;

static const char *col_defs[][15] = { /* FIXME: this is convenient, but not memory efficient. Shouldn't matter. */
	{"header", NULL},
	{"bed", "chrom", "start", "end", "name", "score", "strand", "thickstart", "thickend", "rgb", "blockcount", "blocksizes", "blockstarts", NULL},
	{"sam", "qname", "flag", "rname", "pos", "mapq", "cigar", "rnext", "pnext", "tlen", "seq", "qual", NULL},
	{"vcf", "chrom", "pos", "id", "ref", "alt", "qual", "filter" "info", NULL},
	{"gff", "seqname", "source", "feature", "start", "end", "score", "filter", "strand", "group", "attribute", NULL},
	{NULL}
};

static const char *tab_delim = "nyyyyn", *hdr_chr = "\0#@##\0";

static void set_colnm_aux(const char *p, int col)
{
	const char *q;
	Cell *x;
	for (q = p; *q; ++q)
		if (!isdigit(*q)) break;
	if (*q == 0) return; /* do not set if string p is an integer */
	if ((x = lookup(p, symtab)) != NULL)
		x->tval = NUM, x->fval = col;
}

int bio_get_fmt(const char *s)
{
	int i, j;
	if (strcmp(s, "hdr") == 0) return BIO_HDR;
	for (i = 0; col_defs[i][0]; ++i)
		if (strcmp(s, col_defs[i][0]) == 0) return i;
	for (i = 1; col_defs[i][0]; ++i) {
		printf("%s:\n\t", col_defs[i][0]);
		for (j = 1; col_defs[i][j]; ++j)
			printf("%d:%s ", j, col_defs[i][j]);
		putchar('\n');
	}
	return BIO_NULL;
}

int bio_skip_hdr(const char *r)
{
	if (bio_fmt <= BIO_HDR) return 0;
	if (*r && *r == hdr_chr[bio_fmt]) {
		if (bio_flag & BIO_SHOW_HDR) puts(r);
		return 1;
	} else return 0;
}

void bio_set_colnm()
{
	int i;
	if (bio_fmt == BIO_NULL) {
		return;
	} else if (bio_fmt == BIO_HDR) {
		char *p, *q, c;
		for (p = record; *p && isspace(*p); ++p); /* skip leading spaces */
		for (i = 1, q = p; *q; ++q) {
			if (!isspace(*q)) continue;
			c = *q; /* backup the space */
			*q = 0; /* terminate the field */
			set_colnm_aux(p, i);
			*q = c; /* change back */
			++i;
			for (p = q + 1; *p && isspace(*p); ++p); /* skip contiguous spaces */
			q = p;
		}
		set_colnm_aux(p, i); /* the last column */
	} else {
		for (i = 0; col_defs[bio_fmt][i] != NULL; ++i)
			set_colnm_aux(col_defs[bio_fmt][i], i);
		if (tab_delim[bio_fmt] == 'y') *FS = *OFS = "\t";
	}
}

#define tempfree(x)   if (istemp(x)) tfree(x); else

Cell *bio_func(int f, Cell *x, Node **a)
{
	Cell *y, *z;
	y = gettemp();
	if (f == BIO_FAND) {
		if (a[1]->nnext == 0) {
			WARNING("and requires two arguments; returning 0.0");
			setfval(y, 0.0);
		} else {
			z = execute(a[1]->nnext);
			setfval(y, (Awkfloat)((long)getfval(x) & (long)getfval(z)));
			tempfree(z);
		}
	}
	// else: never happens
	return y;
}
