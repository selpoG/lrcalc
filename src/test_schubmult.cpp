/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/perm.hpp"
#include "lrcalc/schublib.hpp"

#define PROGNAME "test_schubmult"

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

static ivlc_slice get_rank(const ivlc_ptr& lc, int rank)
{
	ivlc_slice res = ivlc_create_slice();

	for (auto& kv : ivlc_iterator(lc))
		if (rank == 0 || perm_group(*kv.key) <= rank) ivlc_insert(*res, *kv.key, kv.hash, kv.value);
	return res;
}

static bool test_mult_schubert(ivector& w1, ivector& w2)
{
	ivlc_ptr prd12;
	{
		ivlc_ptr poly{trans(w1, 0)};
		prd12.reset(mult_poly_schubert(*poly.release(), w2, 0));
	}

	{
		ivlc_ptr prd21;
		{
			ivlc_ptr poly{trans(w2, 0)};
			prd21.reset(mult_poly_schubert(*poly.release(), w1, 0));
		}
		assert(ivlc_equals(*prd12, *prd21));
		prd21.reset();
	}

	int maxrank = 0;
	for (const auto& kv : ivlc_iterator(prd12))
	{
		int r = perm_group(*kv.key);
		if (maxrank < r) maxrank = r;
	}

	for (int r = 0; r <= maxrank; r++)
	{
		ivlc_ptr prd_sm{mult_schubert(w1, w2, r)};

		ivlc_slice prd_gr = get_rank(prd12, r);
		assert(ivlc_equals(*prd_sm, *prd_gr));
	}

	return true;
}

int main(int ac, char** av)
{
	if (ac != 2) print_usage();
	int n = atoi(av[1]);
	if (n < 0) print_usage();

	ivl_ptr lst{all_perms(n)};
	if (!lst) out_of_memory();

	for (uint32_t i = 0; i < ivl_length(lst); i++)
		for (uint32_t j = 0; j < ivl_length(lst); j++)
		{
			ivector* w1 = ivl_elem(lst, i);
			ivector* w2 = ivl_elem(lst, j);
			if (!test_mult_schubert(*w1, *w2)) out_of_memory();
			assert(int(iv_length(w1)) == n && int(iv_length(w2)) == n);
		}

	puts("success");
	return 0;
}
