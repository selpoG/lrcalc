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

#define PROGNAME "test_lrskew"

#include <memory>

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
struct ivlc_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free_all(p); }
};
struct ivlc_slice_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free(p); }
};
using safe_iv_ptr = std::unique_ptr<ivector, iv_deleter>;
using safe_ivlc_ptr = std::unique_ptr<ivlincomb, ivlc_deleter>;
using safe_ivlc_slice = std::unique_ptr<ivlincomb, ivlc_slice_deleter>;

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rows cols\n");
	exit(1);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

static ivlincomb* get_strip(const ivlincomb* lc, int rows)
{
	safe_ivlc_ptr res{ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)};
	if (!res) return nullptr;
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivlc_keyval_t* kv = ivlc_keyval(&itr);
		if (part_entry(kv->key, rows) == 0)
			if (ivlc_insert(res.get(), kv->key, kv->hash, kv->value) == nullptr) return nullptr;
	}
	return res.release();
}

static bool test_schur_lrskew(const ivector* out, const ivector* inn, int rows, int cols)
{
	safe_iv_ptr sh{iv_new(uint32_t(rows))};
	if (!sh) return true;
	safe_ivlc_ptr lc{schur_skew(out, inn, -1, rows)};
	if (!lc) return true;

	/* Check that schur_skew() agrees with schur_lrcoef(). */
	part_iter itr;
	pitr_box_first(&itr, sh.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		long long coef = schur_lrcoef(out, inn, sh.get());
		if (coef < 0) return true;
		const ivlc_keyval_t* kv = ivlc_lookup(lc.get(), sh.get(), iv_hash(sh.get()));
		assert(coef == (kv ? kv->value : 0));
	}

	/* Check results when limiting number of rows. */
	for (int r = 0; r <= rows; r++)
	{
		safe_ivlc_ptr lc_sk{schur_skew(out, inn, r, rows)};
		if (!lc_sk) return true;
		safe_ivlc_slice lc_gs{get_strip(lc.get(), r)};
		if (!lc_gs) return true;
		assert(ivlc_equals(lc_sk.get(), lc_gs.get(), 0));
	}

	return false;
}

int main(int ac, char** av)
{
	alloc_getenv();

	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows < 0 || cols < 0) print_usage();

	safe_iv_ptr out{iv_new(uint32_t(rows))};
	if (out == nullptr) out_of_memory();
	safe_iv_ptr inn{iv_new(uint32_t(rows))};
	if (inn == nullptr) out_of_memory();

	part_iter itr1;
	pitr_box_first(&itr1, out.get(), rows, cols);
	for (; pitr_good(&itr1); pitr_next(&itr1))
	{
		part_iter itr2;
		pitr_box_first(&itr2, inn.get(), rows, cols);
		for (; pitr_good(&itr2); pitr_next(&itr2))
			if (test_schur_lrskew(out.get(), inn.get(), rows, cols)) out_of_memory();
	}

	puts("success");
	alloc_report();
	return 0;
}
