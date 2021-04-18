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

#define PROGNAME "test_lrcoef"

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

int test_schur_lrcoef(ivector* p1, ivector* p2, int rows, int cols)
{
	ivlincomb* prd;
	part_iter itr;
	ivector* outer;

	prd = NULL;
	outer = iv_new(rows);
	if (!outer) goto out_of_mem;

	prd = schur_mult(p1, p2, rows, cols, rows);
	if (!prd) goto out_of_mem;

	pitr_box_first(&itr, outer, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		ivlc_keyval_t* kv;
		int coef = schur_lrcoef(outer, p1, p2);
		if (coef < 0) goto out_of_mem;
		kv = ivlc_lookup(prd, outer, iv_hash(outer));
		assert(coef == (kv ? kv->value : 0));
	}

	iv_free(outer);
	ivlc_free_all(prd);
	return 0;

out_of_mem:
	if (outer) iv_free(outer);
	if (prd) ivlc_free_all(prd);
	return -1;
}

int main(int ac, char** av)
{
	int rows, cols;
	ivector *p1, *p2;
	part_iter itr1, itr2;

	alloc_getenv();

	if (ac != 3) print_usage();
	rows = atoi(av[1]);
	cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	p1 = iv_new(rows);
	if (p1 == NULL) out_of_memory();
	p2 = iv_new(rows);
	if (p2 == NULL)
	{
		iv_free(p1);
		out_of_memory();
	}

	pitr_box_first(&itr1, p1, rows, cols);
	for (; pitr_good(&itr1); pitr_next(&itr1))
	{
		pitr_box_first(&itr2, p2, rows, cols);
		for (; pitr_good(&itr2); pitr_next(&itr2))
		{
			if (test_schur_lrcoef(p1, p2, rows, cols) != 0)
			{
				iv_free(p1);
				iv_free(p2);
				out_of_memory();
			}
		}
	}

	puts("success");
	iv_free(p1);
	iv_free(p2);
	alloc_report();
	return 0;
}
