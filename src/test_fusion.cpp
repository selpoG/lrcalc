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

#define PROGNAME "test_fusion"

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

static bool test_mult_fusion(ivector* sh1, ivector* sh2, int rows, int level)
{
	safe_ivlc_ptr prd_f{schur_mult_fusion(sh1, sh2, rows, level)};
	if (!prd_f) return true;

	safe_ivlc_ptr prd_s{schur_mult(sh1, sh2, rows, -1, rows)};
	if (!prd_s) return true;

	if (fusion_reduce_lc(prd_s.get(), level)) return true;

	assert(ivlc_equals(prd_f.get(), prd_s.get(), 0));

	return false;
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	safe_iv_ptr sh1{iv_new(uint32_t(rows))};
	if (sh1 == nullptr) out_of_memory();
	safe_iv_ptr sh2{iv_new(uint32_t(rows))};
	if (sh2 == nullptr) out_of_memory();

	part_iter itr1;
	pitr_box_first(&itr1, sh1.get(), rows, cols);
	for (; pitr_good(&itr1); pitr_next(&itr1))
	{
		part_iter itr2;
		pitr_box_first(&itr2, sh2.get(), rows, cols);
		for (; pitr_good(&itr2); pitr_next(&itr2))
			for (int level = 0; level <= cols; level++)
				if (test_mult_fusion(sh1.get(), sh2.get(), rows, level)) out_of_memory();
	}

	puts("success");
	return 0;
}
