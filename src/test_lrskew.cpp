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

#define PROGNAME "test_lrskew"

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

static ivlc_slice get_strip(const ivlc_ptr& lc, int rows)
{
	ivlc_slice res = ivlc_create_slice();
	if (!res) return res;
	for (auto& kv : ivlc_iterator(lc))
		if (part_entry(kv.key, rows) == 0)
			if (ivlc_insert(res.get(), kv.key, kv.hash, kv.value) == nullptr) return nullptr;
	return res;
}

static bool test_schur_lrskew(const iv_ptr& out, const iv_ptr& inn, int rows, int cols)
{
	iv_ptr sh = iv_create(uint32_t(rows));
	ivlc_ptr lc{schur_skew(out.get(), inn.get(), -1, rows)};
	if (!lc) return false;

	/* Check that schur_skew() agrees with schur_lrcoef(). */
	for ([[maybe_unused]] auto& itr : pitr::box(sh.get(), rows, cols))
	{
		long long coef = schur_lrcoef(out.get(), inn.get(), sh.get());
		if (coef < 0) return false;
		[[maybe_unused]] const ivlc_keyval_t* kv = ivlc_lookup(lc.get(), sh.get(), iv_hash(sh.get()));
		assert(coef == (kv ? kv->value : 0));
	}

	/* Check results when limiting number of rows. */
	for (int r = 0; r <= rows; r++)
	{
		ivlc_ptr lc_sk{schur_skew(out.get(), inn.get(), r, rows)};
		if (!lc_sk) return false;
		ivlc_slice lc_gs = get_strip(lc, r);
		if (!lc_gs) return false;
		assert(ivlc_equals(lc_sk.get(), lc_gs.get()));
	}

	return true;
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	iv_ptr out = iv_create(uint32_t(rows));
	iv_ptr inn = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr1 : pitr::box(out.get(), rows, cols))
		for ([[maybe_unused]] auto& itr2 : pitr::box(inn.get(), rows, cols))
			if (!test_schur_lrskew(out, inn, rows, cols)) out_of_memory();

	puts_r("success");
	return 0;
}
