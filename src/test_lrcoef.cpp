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
#include "lrcalc/part.hpp"
#include "lrcalc/schur.hpp"

#define PROGNAME "test_lrcoef"

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
struct ivlc_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free_all(p); }
};
using safe_iv_ptr = std::unique_ptr<ivector, iv_deleter>;
using safe_ivlc_ptr = std::unique_ptr<ivlincomb, ivlc_deleter>;

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rows cols\n");
	exit(1);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

static bool test_schur_lrcoef(const ivector* p1, const ivector* p2, int rows, int cols)
{
	safe_iv_ptr outer{iv_new(uint32_t(rows))};
	if (!outer) return true;

	safe_ivlc_ptr prd{schur_mult(p1, p2, rows, cols, rows)};
	if (!prd) return true;

	part_iter itr;
	pitr_box_first(&itr, outer.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		long long coef = schur_lrcoef(outer.get(), p1, p2);
		if (coef < 0) return true;
		[[maybe_unused]] const ivlc_keyval_t* kv = ivlc_lookup(prd.get(), outer.get(), iv_hash(outer.get()));
		assert(coef == (kv ? kv->value : 0));
	}

	return false;
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	safe_iv_ptr p1{iv_new(uint32_t(rows))};
	if (!p1) out_of_memory();
	safe_iv_ptr p2{iv_new(uint32_t(rows))};
	if (!p2) out_of_memory();

	part_iter itr1;
	pitr_box_first(&itr1, p1.get(), rows, cols);
	for (; pitr_good(&itr1); pitr_next(&itr1))
	{
		part_iter itr2;
		pitr_box_first(&itr2, p2.get(), rows, cols);
		for (; pitr_good(&itr2); pitr_next(&itr2))
			if (test_schur_lrcoef(p1.get(), p2.get(), rows, cols)) out_of_memory();
	}

	puts("success");
	return 0;
}
