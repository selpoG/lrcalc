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
	if (!outer) return true;

	ivlc_ptr prd{schur_mult(p1.get(), p2.get(), rows, cols, rows)};
	if (!prd) return true;

	for ([[maybe_unused]] auto& itr : pitr::box(outer.get(), rows, cols))
	{
		long long coef = schur_lrcoef(outer.get(), p1.get(), p2.get());
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

	iv_ptr p1 = iv_create(uint32_t(rows));
	if (!p1) out_of_memory();
	iv_ptr p2 = iv_create(uint32_t(rows));
	if (!p2) out_of_memory();

	for ([[maybe_unused]] auto& itr1 : pitr::box(p1.get(), rows, cols))
		for ([[maybe_unused]] auto& itr2 : pitr::box(p2.get(), rows, cols))
			if (test_schur_lrcoef(p1, p2, rows, cols)) out_of_memory();

	puts("success");
	return 0;
}
