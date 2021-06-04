/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

#define PROGNAME "test_partiter"

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory\n");
	exit(1);
}

static bool test_part_iter_box(int rows, int cols)
{
	iv_ptr p = iv_create(uint32_t(rows));

	int np = 1;
	for (int i = 1; i <= rows; i++) np = np * (cols + i) / i;

	int np1 = 0;
	for ([[maybe_unused]] auto& itr : pitr::box(*p, rows, cols))
	{
		assert(part_valid(*p));
		np1++;
	}
	assert(np1 == np);

	np1 = 0;
	for (int size = 0; size < rows * cols + 2; size++)
		for ([[maybe_unused]] auto& itr : pitr::box_sz(*p, rows, cols, size))
		{
			assert(part_valid(*p));
			assert(iv_sum(*p) == size);
			np1++;
		}
	assert(np1 == np);
	return true;
}

static bool test_part_iter_sub(int rows, int cols, const iv_ptr& outer)
{
	int size_bound = iv_sum(*outer) + 2;
	iv_ptr count = iv_create_zero(uint32_t(size_bound));

	iv_ptr p = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr : pitr::box(*p, rows, cols))
		if (part_leq(*p, *outer))
		{
			int sz = iv_sum(*p);
			iv_elem(count, sz)++;
		}

	int np = 0;
	for ([[maybe_unused]] auto& itr : pitr(*p, rows, cols, outer.get(), nullptr, 0, PITR_USE_OUTER))
	{
		assert(part_valid(*p));
		assert(part_leq(*p, *outer));
		np++;
	}
	assert(np == iv_sum(*count));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		for ([[maybe_unused]] auto& itr :
		     pitr(*p, rows, cols, outer.get(), nullptr, size, PITR_USE_OUTER | PITR_USE_SIZE))
		{
			assert(part_valid(*p));
			assert(part_leq(*p, *outer));
			assert(iv_sum(*p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return true;
}

static bool test_part_iter_super(int rows, int cols, const iv_ptr& inner)
{
	int size_bound = rows * cols + 2;
	iv_ptr count = iv_create_zero(uint32_t(size_bound));

	iv_ptr p = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr : pitr::box(*p, rows, cols))
		if (part_leq(*inner, *p))
		{
			int sz = iv_sum(*p);
			iv_elem(count, sz)++;
		}

	int np = 0;
	for ([[maybe_unused]] auto& itr : pitr(*p, rows, cols, nullptr, inner.get(), 0, PITR_USE_INNER))
	{
		assert(part_valid(*p));
		assert(part_leq(*inner, *p));
		np++;
	}
	assert(np == iv_sum(*count));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		for ([[maybe_unused]] auto& itr :
		     pitr(*p, rows, cols, nullptr, inner.get(), size, PITR_USE_INNER | PITR_USE_SIZE))
		{
			assert(part_valid(*p));
			assert(part_leq(*inner, *p));
			assert(iv_sum(*p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return true;
}

static bool test_part_iter_between(int rows, int cols, const iv_ptr& outer, const iv_ptr& inner)
{
	int size_bound = iv_sum(*outer) + 2;
	iv_ptr count = iv_create_zero(uint32_t(size_bound));

	iv_ptr p = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr : pitr::box(*p, rows, cols))
		if (part_leq(*inner, *p) && part_leq(*p, *outer))
		{
			int sz = iv_sum(*p);
			iv_elem(count, sz)++;
		}

	int np = 0;
	for ([[maybe_unused]] auto& itr :
	     pitr(*p, rows, cols, outer.get(), inner.get(), 0, PITR_USE_OUTER | PITR_USE_INNER))
	{
		assert(part_valid(*p));
		assert(part_leq(*inner, *p));
		assert(part_leq(*p, *outer));
		np++;
	}
	assert(np == iv_sum(*count));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		for ([[maybe_unused]] auto& itr :
		     pitr(*p, rows, cols, outer.get(), inner.get(), size, PITR_USE_OUTER | PITR_USE_INNER | PITR_USE_SIZE))
		{
			assert(part_valid(*p));
			assert(part_leq(*inner, *p));
			assert(part_leq(*p, *outer));
			assert(iv_sum(*p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return true;
}

int main(int ac, char** av)
{
	if (ac != 3)
	{
		fprintf(stderr, "usage: " PROGNAME " rows cols\n");
		exit(1);
	}
	int rows = int(atol(av[1]));
	int cols = int(atol(av[2]));
	int rows0 = rows ? rows - 1 : 0;
	int cols0 = cols ? cols - 1 : 0;

	iv_ptr p1 = iv_create(uint32_t(rows));
	iv_ptr p2 = iv_create(uint32_t(rows));

	if (!test_part_iter_box(rows, cols)) out_of_memory();

	for ([[maybe_unused]] auto& itr_p2 : pitr::box(*p2, rows, cols))
	{
		uint32_t p2_len = iv_length(p2);
		part_unchop(*p2, rows);

		if (!test_part_iter_sub(rows, cols, p2)) out_of_memory();
		if (!test_part_iter_sub(rows0, cols, p2)) out_of_memory();
		if (!test_part_iter_sub(rows, cols0, p2)) out_of_memory();
		if (!test_part_iter_sub(rows, cols + 1, p2)) out_of_memory();
		if (!test_part_iter_super(rows, cols, p2)) out_of_memory();
		if (!test_part_iter_super(rows0, cols, p2)) out_of_memory();
		if (!test_part_iter_super(rows, cols0, p2)) out_of_memory();
		if (!test_part_iter_super(rows, cols + 2, p2)) out_of_memory();

		for ([[maybe_unused]] auto& itr_p1 : pitr(*p1, rows, cols, p2.get(), nullptr, 0, PITR_USE_OUTER))
		{
			uint32_t p1_len = iv_length(p1);
			part_unchop(*p1, rows);

			if (!test_part_iter_between(rows, cols, p2, p1)) out_of_memory();
			if (!test_part_iter_between(rows0, cols, p2, p1)) out_of_memory();
			if (!test_part_iter_between(rows, cols0, p2, p1)) out_of_memory();
			if (!test_part_iter_between(rows, cols + 2, p2, p1)) out_of_memory();

			iv_length(p1) = p1_len;
		}
		iv_length(p2) = p2_len;
	}

	puts("success");
	return 0;
}
