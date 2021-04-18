/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "alloc.h"
#include "ivlincomb.h"
#include "part.h"
#include "perm.h"
#include "schublib.h"
#include "schur.h"

#define PROGNAME "test_lrskew"

void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rows cols\n");
	exit(1);
}

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

ivlincomb* get_strip(ivlincomb* lc, int rows)
{
	ivlincomb* res;
	ivlc_iter itr;
	res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == NULL) return NULL;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivlc_keyval_t* kv = ivlc_keyval(&itr);
		if (part_entry(kv->key, rows) == 0)
		{
			if (ivlc_insert(res, kv->key, kv->hash, kv->value) == NULL)
			{
				ivlc_free(res);
				return NULL;
			}
		}
	}
	return res;
}

int test_schur_lrskew(ivector* out, ivector* inn, int rows, int cols)
{
	ivlincomb *lc, *lc_sk, *lc_gs;
	part_iter itr;
	ivector* sh;
	int r;

	lc = lc_sk = lc_gs = NULL;
	sh = iv_new(rows);
	if (!sh) goto out_of_mem;
	lc = schur_skew(out, inn, -1, rows);
	if (!lc) goto out_of_mem;

	/* Check that schur_skew() agrees with schur_lrcoef(). */
	pitr_box_first(&itr, sh, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		ivlc_keyval_t* kv;
		int coef = schur_lrcoef(out, inn, sh);
		if (coef < 0) goto out_of_mem;
		kv = ivlc_lookup(lc, sh, iv_hash(sh));
		assert(coef == (kv ? kv->value : 0));
	}

	/* Check results when limiting number of rows. */
	for (r = 0; r <= rows; r++)
	{
		lc_sk = schur_skew(out, inn, r, rows);
		if (!lc_sk) goto out_of_mem;
		lc_gs = get_strip(lc, r);
		if (!lc_gs) goto out_of_mem;
		assert(ivlc_equals(lc_sk, lc_gs, 0));
		ivlc_free_all(lc_sk);
		ivlc_free(lc_gs);
		lc_sk = lc_gs = NULL;
	}

	iv_free(sh);
	ivlc_free_all(lc);
	return 0;

out_of_mem:
	if (sh) iv_free(sh);
	if (lc) ivlc_free_all(lc);
	if (lc_sk) ivlc_free_all(lc_sk);
	if (lc_gs) ivlc_free(lc_gs);
	return -1;
}

int main(int ac, char** av)
{
	int rows, cols;
	ivector *out, *inn;
	part_iter itr1, itr2;

	alloc_getenv();

	if (ac != 3) print_usage();
	rows = atoi(av[1]);
	cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	out = iv_new(rows);
	if (out == NULL) out_of_memory();
	inn = iv_new(rows);
	if (inn == NULL)
	{
		iv_free(out);
		out_of_memory();
	}

	pitr_box_first(&itr1, out, rows, cols);
	for (; pitr_good(&itr1); pitr_next(&itr1))
	{
		pitr_box_first(&itr2, inn, rows, cols);
		for (; pitr_good(&itr2); pitr_next(&itr2))
		{
			if (test_schur_lrskew(out, inn, rows, cols) != 0)
			{
				iv_free(out);
				iv_free(inn);
				out_of_memory();
			}
		}
	}

	puts("success");
	iv_free(out);
	iv_free(inn);
	alloc_report();
	return 0;
}
