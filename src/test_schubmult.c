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
#include "ivlist.h"
#include "perm.h"
#include "schublib.h"

#define PROGNAME "test_schubmult"

void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rank\n");
	exit(1);
}

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

ivlincomb* get_rank(ivlincomb* lc, int rank)
{
	ivlc_iter itr;
	ivlincomb* res;

	res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == NULL) return NULL;

	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
		if (rank == 0 || perm_group(ivlc_key(&itr)) <= rank)
		{
			ivlc_keyval_t* kv = ivlc_keyval(&itr);
			if (ivlc_insert(res, kv->key, kv->hash, kv->value) == NULL)
			{
				ivlc_free(res);
				return NULL;
			}
		}
	return res;
}

int test_mult_schubert(ivector* w1, ivector* w2)
{
	ivlincomb *poly, *prd12, *prd21, *prd_sm, *prd_gr;
	ivlc_iter itr;
	int maxrank, r;

	prd12 = prd21 = prd_sm = prd_gr = NULL;

	poly = trans(w1, 0);
	if (poly == NULL) goto out_of_mem;
	prd12 = mult_poly_schubert(poly, w2, 0);
	if (prd12 == NULL) goto out_of_mem;

	poly = trans(w2, 0);
	if (poly == NULL) goto out_of_mem;
	prd21 = mult_poly_schubert(poly, w1, 0);
	if (prd21 == NULL) goto out_of_mem;

	assert(ivlc_equals(prd12, prd21, 0));
	ivlc_free_all(prd21);
	prd21 = NULL;

	maxrank = 0;
	for (ivlc_first(prd12, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		r = perm_group(ivlc_key(&itr));
		if (maxrank < r) maxrank = r;
	}

	for (r = 0; r <= maxrank; r++)
	{
		prd_sm = mult_schubert(w1, w2, r);
		if (prd_sm == NULL) goto out_of_mem;

		prd_gr = get_rank(prd12, r);
		if (prd_gr == NULL) goto out_of_mem;
		assert(ivlc_equals(prd_sm, prd_gr, 0));
		ivlc_free_all(prd_sm);
		ivlc_free(prd_gr);
		prd_sm = prd_gr = NULL;
	}

	ivlc_free_all(prd12);
	return 0;

out_of_mem:
	if (prd12) ivlc_free_all(prd12);
	if (prd21) ivlc_free_all(prd21);
	if (prd_sm) ivlc_free_all(prd_sm);
	if (prd_gr) ivlc_free(prd_gr);
	return -1;
}

int main(int ac, char** av)
{
	ivlist* lst;
	int n, i, j;

	alloc_getenv();

	if (ac != 2) print_usage();
	n = atoi(av[1]);
	if (n < 0) print_usage();

	lst = all_perms(n);
	if (lst == NULL) out_of_memory();

	for (i = 0; i < ivl_length(lst); i++)
		for (j = 0; j < ivl_length(lst); j++)
		{
			ivector *w1, *w2;
			w1 = ivl_elem(lst, i);
			w2 = ivl_elem(lst, j);
			if (test_mult_schubert(w1, w2) != 0)
			{
				ivl_free_all(lst);
				out_of_memory();
			}
			assert(iv_length(w1) == n && iv_length(w2) == n);
		}

	puts("success");
	ivl_free_all(lst);
	alloc_report();
	return 0;
}
