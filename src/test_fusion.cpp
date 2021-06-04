/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/schur.hpp"

#define PROGNAME "test_fusion"

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

static bool test_mult_fusion(const iv_ptr& sh1, const iv_ptr& sh2, int rows, int level)
{
	ivlc_ptr prd_f{schur_mult_fusion(*sh1, *sh2, rows, level)};
	if (!prd_f) return false;

	ivlc_ptr prd_s{schur_mult(*sh1, sh2.get(), rows, -1, rows)};
	if (!prd_s) return false;

	fusion_reduce_lc(*prd_s, level);

	assert(ivlc_equals(*prd_f, *prd_s));

	return true;
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	iv_ptr sh1 = iv_create(uint32_t(rows));
	iv_ptr sh2 = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr1 : pitr::box(*sh1, rows, cols))
		for ([[maybe_unused]] auto& itr2 : pitr::box(*sh2, rows, cols))
			for (int level = 0; level <= cols; level++)
				if (!test_mult_fusion(sh1, sh2, rows, level)) out_of_memory();

	puts("success");
	return 0;
}
