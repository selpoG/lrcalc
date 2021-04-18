/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/perm.hpp"
#include "lrcalc/schublib.hpp"
#include "lrcalc/schur.hpp"

#define PROGNAME "test_lrmult"

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

ivector* part2string(ivector* p, int rows, int cols)
{
	int i;
	ivector* s = iv_new(rows + cols);
	if (s == NULL) return NULL;
	for (i = 0; i < iv_length(s); i++) iv_elem(s, i) = 1;
	for (i = 0; i < rows; i++) iv_elem(s, i + part_entry(p, rows - 1 - i)) = 0;
	return s;
}

ivector* string2part(ivector* s, int rows)
{
	int i, j;
	ivector* p = iv_new(rows);
	if (p == NULL) return NULL;
	i = 0;
	for (j = 0; j < iv_length(s); j++)
		if (iv_elem(s, j) == 0)
		{
			iv_elem(p, rows - 1 - i) = j - i;
			i++;
		}
	return p;
}

ivlincomb* string2part_lc(ivlincomb* lc, int rows)
{
	ivlincomb* res;
	ivlc_iter itr;
	res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == NULL) return NULL;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivector* p = string2part(ivlc_key(&itr), rows);
		if (p == NULL)
		{
			ivlc_free_all(res);
			return NULL;
		}
		if (ivlc_insert(res, p, iv_hash(p), ivlc_value(&itr)) == NULL)
		{
			iv_free(p);
			ivlc_free_all(res);
			return NULL;
		}
	}
	return res;
}

ivlincomb* get_box(ivlincomb* lc, int rows, int cols)
{
	ivlincomb* res;
	ivlc_iter itr;
	res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == NULL) return NULL;
	if (rows == -1) rows = (((unsigned)-1) >> 1);
	if (cols == -1) cols = (((unsigned)-1) >> 1);
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivlc_keyval_t* kv = ivlc_keyval(&itr);
		if (part_entry(kv->key, rows) == 0 && part_entry(kv->key, 0) <= cols)
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

int test_schur_mult(ivector* p1, ivector* p2)
{
	ivector *s1, *s2;
	ivlincomb *prd_s, *prd, *prd_sm, *prd_gb;
	int rows, cols, r, c;

	rows = part_length(p1) + part_length(p2);
	cols = part_entry(p1, 0) + part_entry(p2, 0);

	s1 = s2 = NULL;
	prd_s = prd = prd_sm = prd_gb = NULL;

	s1 = part2string(p1, rows, cols);
	if (!s1) goto out_of_mem;
	s2 = part2string(p2, rows, cols);
	if (!s2) goto out_of_mem;
	prd_s = mult_schubert_str(s1, s2);
	if (!prd_s) goto out_of_mem;
	iv_free(s1);
	iv_free(s2);
	s1 = s2 = NULL;
	prd = string2part_lc(prd_s, rows);
	if (!prd) goto out_of_mem;
	ivlc_free_all(prd_s);
	prd_s = NULL;

	prd_sm = schur_mult(p1, p2, -1, -1, -1);
	if (!prd_sm) goto out_of_mem;
	assert(ivlc_equals(prd_sm, prd, 0));
	ivlc_free_all(prd_sm);
	prd_sm = NULL;

	for (r = -1; r <= rows; r++)
		for (c = -1; c <= cols; c++)
		{
			prd_sm = schur_mult(p1, p2, r, c, rows);
			if (!prd_sm) goto out_of_mem;
			prd_gb = get_box(prd, r, c);
			if (!prd_gb) goto out_of_mem;
			assert(ivlc_equals(prd_sm, prd_gb, 0));
			ivlc_free_all(prd_sm);
			ivlc_free(prd_gb);
			prd_sm = prd_gb = NULL;
		}

	ivlc_free_all(prd);
	return 0;

out_of_mem:
	if (s1) iv_free(s1);
	if (s2) iv_free(s2);
	if (prd_s) ivlc_free_all(prd_s);
	if (prd) ivlc_free_all(prd);
	if (prd_sm) ivlc_free_all(prd_sm);
	if (prd_gb) ivlc_free(prd_gb);
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
			if (test_schur_mult(p1, p2) != 0)
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
