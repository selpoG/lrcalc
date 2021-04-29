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

#define PROGNAME "test_lrmult"

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

static ivector* part2string(ivector* p, int rows, int cols)
{
	safe_iv_ptr s{iv_new(uint32_t(rows + cols))};
	if (!s) return nullptr;
	for (int i = 0; uint32_t(i) < iv_length(s); i++) iv_elem(s, i) = 1;
	for (int i = 0; i < rows; i++) iv_elem(s, i + part_entry(p, rows - 1 - i)) = 0;
	return s.release();
}

static ivector* string2part(ivector* s, int rows)
{
	safe_iv_ptr p{iv_new(uint32_t(rows))};
	if (!p) return nullptr;
	int i = 0;
	for (int j = 0; uint32_t(j) < iv_length(s); j++)
		if (iv_elem(s, j) == 0)
		{
			iv_elem(p, rows - 1 - i) = j - i;
			i++;
		}
	return p.release();
}

static ivlincomb* string2part_lc(ivlincomb* lc, int rows)
{
	ivlc_iter itr;
	safe_ivlc_ptr res{ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)};
	if (!res) return nullptr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		safe_iv_ptr p{string2part(ivlc_key(&itr), rows)};
		if (!p) return nullptr;
		if (ivlc_insert(res.get(), p.get(), iv_hash(p.get()), ivlc_value(&itr)) == nullptr) return nullptr;
		p.release();
	}
	return res.release();
}

static ivlincomb* get_box(ivlincomb* lc, int rows, int cols)
{
	safe_ivlc_ptr res{ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)};
	if (!res) return nullptr;
	if (rows == -1) rows = (unsigned(-1) >> 1);
	if (cols == -1) cols = (unsigned(-1) >> 1);
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivlc_keyval_t* kv = ivlc_keyval(&itr);
		if (part_entry(kv->key, rows) == 0 && part_entry(kv->key, 0) <= cols)
			if (ivlc_insert(res.get(), kv->key, kv->hash, kv->value) == nullptr) return nullptr;
	}
	return res.release();
}

static bool test_schur_mult(ivector* p1, ivector* p2)
{
	auto rows = int(part_length(p1) + part_length(p2));
	int cols = part_entry(p1, 0) + part_entry(p2, 0);

	safe_ivlc_ptr prd;
	{
		safe_ivlc_ptr prd_s;
		{
			safe_iv_ptr s1{part2string(p1, rows, cols)};
			if (!s1) return true;
			safe_iv_ptr s2{part2string(p2, rows, cols)};
			if (!s2) return true;
			prd_s.reset(mult_schubert_str(s1.get(), s2.get()));
			if (!prd_s) return true;
		}
		prd.reset(string2part_lc(prd_s.get(), rows));
		if (!prd) return true;
	}

	{
		safe_ivlc_ptr prd_sm{schur_mult(p1, p2, -1, -1, -1)};
		if (!prd_sm) return true;
		assert(ivlc_equals(prd_sm.get(), prd.get(), 0));
	}

	for (int r = -1; r <= rows; r++)
		for (int c = -1; c <= cols; c++)
		{
			safe_ivlc_ptr prd_sm{schur_mult(p1, p2, r, c, rows)};
			if (!prd_sm) return true;
			safe_ivlc_slice prd_gb{get_box(prd.get(), r, c)};
			if (!prd_gb) return true;
			assert(ivlc_equals(prd_sm.get(), prd_gb.get(), 0));
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
			if (test_schur_mult(p1.get(), p2.get())) out_of_memory();
	}

	puts("success");
	alloc_report();
	return 0;
}
