/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <limits>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/schublib.hpp"
#include "lrcalc/schur.hpp"

#define PROGNAME "test_lrmult"

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

static iv_ptr part2string(const iv_ptr& p, int rows, int cols)
{
	iv_ptr s = iv_create(uint32_t(rows + cols));
	if (!s) return s;
	for (int i = 0; uint32_t(i) < iv_length(s); i++) iv_elem(s, i) = 1;
	for (int i = 0; i < rows; i++) iv_elem(s, i + part_entry(p.get(), rows - 1 - i)) = 0;
	return s;
}

static iv_ptr string2part(const ivector* s, int rows)
{
	iv_ptr p = iv_create(uint32_t(rows));
	if (!p) return p;
	int i = 0;
	for (int j = 0; uint32_t(j) < iv_length(s); j++)
		if (iv_elem(s, j) == 0)
		{
			iv_elem(p, rows - 1 - i) = j - i;
			i++;
		}
	return p;
}

static ivlc_ptr string2part_lc(const ivlc_ptr& lc, int rows)
{
	ivlc_ptr res = ivlc_create();
	if (!res) return res;
	for (const auto& kv : ivlc_iterator(lc))
	{
		iv_ptr p = string2part(kv.key, rows);
		if (!p) return nullptr;
		if (ivlc_insert(res.get(), p.get(), iv_hash(p.get()), kv.value) == nullptr) return nullptr;
		p.release();
	}
	return res;
}

static ivlc_slice get_box(const ivlc_ptr& lc, int rows, int cols)
{
	ivlc_slice res = ivlc_create_slice();
	if (!res) return res;
	if (rows == -1) rows = std::numeric_limits<int>::max();
	if (cols == -1) cols = std::numeric_limits<int>::max();
	for (auto& kv : ivlc_iterator(lc))
		if (part_entry(kv.key, rows) == 0 && part_entry(kv.key, 0) <= cols)
			if (ivlc_insert(res.get(), kv.key, kv.hash, kv.value) == nullptr) return nullptr;
	return res;
}

static bool test_schur_mult(const iv_ptr& p1, const iv_ptr& p2)
{
	auto rows = int(part_length(p1.get()) + part_length(p2.get()));
	int cols = part_entry(p1.get(), 0) + part_entry(p2.get(), 0);

	ivlc_ptr prd;
	{
		ivlc_ptr prd_s;
		{
			iv_ptr s1 = part2string(p1, rows, cols);
			if (!s1) return true;
			iv_ptr s2 = part2string(p2, rows, cols);
			if (!s2) return true;
			prd_s.reset(mult_schubert_str(s1.get(), s2.get()));
			if (!prd_s) return true;
		}
		prd = string2part_lc(prd_s, rows);
		if (!prd) return true;
	}

	{
		ivlc_ptr prd_sm{schur_mult(p1.get(), p2.get(), -1, -1, -1)};
		if (!prd_sm) return true;
		assert(ivlc_equals(prd_sm.get(), prd.get(), 0));
	}

	for (int r = -1; r <= rows; r++)
		for (int c = -1; c <= cols; c++)
		{
			ivlc_ptr prd_sm{schur_mult(p1.get(), p2.get(), r, c, rows)};
			if (!prd_sm) return true;
			ivlc_slice prd_gb = get_box(prd, r, c);
			if (!prd_gb) return true;
			assert(ivlc_equals(prd_sm.get(), prd_gb.get(), 0));
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
			if (test_schur_mult(p1, p2)) out_of_memory();

	puts("success");
	return 0;
}
