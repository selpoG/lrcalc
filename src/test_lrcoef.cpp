/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/schur.hpp"

#define PROGNAME "test_lrcoef"

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

static bool test_schur_lrcoef(const iv_ptr& p1, const iv_ptr& p2, int rows, int cols)
{
	iv_ptr outer = iv_create(uint32_t(rows));

	ivlc_ptr prd{schur_mult(*p1, p2.get(), rows, cols, rows)};
	if (!prd) return false;

	for ([[maybe_unused]] auto& itr : pitr::box(*outer, rows, cols))
	{
		long long coef = schur_lrcoef(*outer, *p1, *p2);
		if (coef < 0) return false;
		[[maybe_unused]] const ivlc_keyval_t* kv = ivlc_lookup(*prd, *outer, iv_hash(*outer));
		assert(coef == (kv ? kv->value : 0));
	}

	return true;
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	iv_ptr p1 = iv_create(uint32_t(rows));
	iv_ptr p2 = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr1 : pitr::box(*p1, rows, cols))
		for ([[maybe_unused]] auto& itr2 : pitr::box(*p2, rows, cols))
			if (!test_schur_lrcoef(p1, p2, rows, cols)) out_of_memory();

	puts("success");
	return 0;
}
