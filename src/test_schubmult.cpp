/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/perm.hpp"
#include "lrcalc/schublib.hpp"

#define PROGNAME "test_schubmult"

struct ivl_deleter
{
	void operator()(ivlist* p) const { ivl_free_all(p); }
};
struct ivlc_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free_all(p); }
};
struct ivlc_slice_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free(p); }
};
using safe_ivl_ptr = std::unique_ptr<ivlist, ivl_deleter>;
using safe_ivlc_ptr = std::unique_ptr<ivlincomb, ivlc_deleter>;
using safe_ivlc_slice = std::unique_ptr<ivlincomb, ivlc_slice_deleter>;

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rank\n");
	exit(1);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

static ivlincomb* get_rank(const ivlincomb* lc, int rank)
{
	safe_ivlc_slice res{ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)};
	if (!res) return nullptr;

	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
		if (rank == 0 || perm_group(ivlc_key(&itr)) <= rank)
		{
			ivlc_keyval_t* kv = ivlc_keyval(&itr);
			if (ivlc_insert(res.get(), kv->key, kv->hash, kv->value) == nullptr) return nullptr;
		}
	return res.release();
}

static bool test_mult_schubert(ivector* w1, ivector* w2)
{
	safe_ivlc_ptr prd12;
	{
		safe_ivlc_ptr poly{trans(w1, 0)};
		if (!poly) return true;
		prd12.reset(mult_poly_schubert(poly.release(), w2, 0));
		if (!prd12) return true;
	}

	{
		safe_ivlc_ptr prd21;
		{
			safe_ivlc_ptr poly{trans(w2, 0)};
			if (!poly) return true;
			prd21.reset(mult_poly_schubert(poly.release(), w1, 0));
			if (!prd21) return true;
		}
		assert(ivlc_equals(prd12.get(), prd21.get(), 0));
		prd21.reset();
	}

	int maxrank = 0;
	ivlc_iter itr;
	for (ivlc_first(prd12.get(), &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		int r = perm_group(ivlc_key(&itr));
		if (maxrank < r) maxrank = r;
	}

	for (int r = 0; r <= maxrank; r++)
	{
		safe_ivlc_ptr prd_sm{mult_schubert(w1, w2, r)};
		if (!prd_sm) return true;

		safe_ivlc_slice prd_gr{get_rank(prd12.get(), r)};
		if (!prd_gr) return true;
		assert(ivlc_equals(prd_sm.get(), prd_gr.get(), 0));
	}

	return false;
}

int main(int ac, char** av)
{
	if (ac != 2) print_usage();
	int n = atoi(av[1]);
	if (n < 0) print_usage();

	safe_ivl_ptr lst{all_perms(n)};
	if (lst == nullptr) out_of_memory();

	for (uint32_t i = 0; i < ivl_length(lst); i++)
		for (uint32_t j = 0; j < ivl_length(lst); j++)
		{
			ivector* w1 = ivl_elem(lst, i);
			ivector* w2 = ivl_elem(lst, j);
			if (test_mult_schubert(w1, w2)) out_of_memory();
			assert(int(iv_length(w1)) == n && int(iv_length(w2)) == n);
		}

	puts("success");
	return 0;
}
