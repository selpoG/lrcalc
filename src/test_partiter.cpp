/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdio.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

#define PROGNAME "test_partiter"

#include <memory>

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
using safe_iv_ptr = std::unique_ptr<ivector, iv_deleter>;

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory\n");
	exit(1);
}

static bool test_part_iter_box(int rows, int cols)
{
	safe_iv_ptr p{iv_new(uint32_t(rows))};
	if (!p) return true;

	int np = 1;
	for (int i = 1; i <= rows; i++) np = np * (cols + i) / i;

	int np1 = 0;
	part_iter itr;
	pitr_box_first(&itr, p.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p.get()));
		np1++;
	}
	assert(np1 == np);

	np1 = 0;
	for (int size = 0; size < rows * cols + 2; size++)
	{
		pitr_box_sz_first(&itr, p.get(), rows, cols, size);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p.get()));
			assert(iv_sum(p.get()) == size);
			np1++;
		}
	}
	assert(np1 == np);
	return false;
}

static bool test_part_iter_sub(int rows, int cols, const ivector* outer)
{
	int size_bound = iv_sum(outer) + 2;
	safe_iv_ptr count{iv_new_zero(uint32_t(size_bound))};
	if (!count) return true;

	safe_iv_ptr p{iv_new(uint32_t(rows))};
	if (!p) return true;

	part_iter itr;
	pitr_box_first(&itr, p.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(p.get(), outer))
		{
			int sz = iv_sum(p.get());
			iv_elem(count, sz)++;
		}

	int np = 0;
	pitr_first(&itr, p.get(), rows, cols, outer, nullptr, 0, PITR_USE_OUTER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p.get()));
		assert(part_leq(p.get(), outer));
		np++;
	}
	assert(np == iv_sum(count.get()));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p.get(), rows, cols, outer, nullptr, size, PITR_USE_OUTER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p.get()));
			assert(part_leq(p.get(), outer));
			assert(iv_sum(p.get()) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return false;
}

static bool test_part_iter_super(int rows, int cols, const ivector* inner)
{
	int size_bound = rows * cols + 2;
	safe_iv_ptr count{iv_new_zero(uint32_t(size_bound))};
	if (!count) return true;

	safe_iv_ptr p{iv_new(uint32_t(rows))};
	if (!p) return true;

	part_iter itr;
	pitr_box_first(&itr, p.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(inner, p.get()))
		{
			int sz = iv_sum(p.get());
			iv_elem(count, sz)++;
		}

	int np = 0;
	pitr_first(&itr, p.get(), rows, cols, nullptr, inner, 0, PITR_USE_INNER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p.get()));
		assert(part_leq(inner, p.get()));
		np++;
	}
	assert(np == iv_sum(count.get()));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p.get(), rows, cols, nullptr, inner, size, PITR_USE_INNER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p.get()));
			assert(part_leq(inner, p.get()));
			assert(iv_sum(p.get()) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return false;
}

static bool test_part_iter_between(int rows, int cols, const ivector* outer, const ivector* inner)
{
	int size_bound = iv_sum(outer) + 2;
	safe_iv_ptr count{iv_new_zero(uint32_t(size_bound))};
	if (!count) return true;

	safe_iv_ptr p{iv_new(uint32_t(rows))};
	if (!p) return true;

	part_iter itr;
	pitr_box_first(&itr, p.get(), rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(inner, p.get()) && part_leq(p.get(), outer))
		{
			int sz = iv_sum(p.get());
			iv_elem(count, sz)++;
		}

	int np = 0;
	pitr_first(&itr, p.get(), rows, cols, outer, inner, 0, PITR_USE_OUTER | PITR_USE_INNER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p.get()));
		assert(part_leq(inner, p.get()));
		assert(part_leq(p.get(), outer));
		np++;
	}
	assert(np == iv_sum(count.get()));

	for (int size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p.get(), rows, cols, outer, inner, size, PITR_USE_OUTER | PITR_USE_INNER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p.get()));
			assert(part_leq(inner, p.get()));
			assert(part_leq(p.get(), outer));
			assert(iv_sum(p.get()) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	return false;
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

	safe_iv_ptr p1{iv_new(uint32_t(rows))};
	if (!p1) out_of_memory();
	safe_iv_ptr p2{iv_new(uint32_t(rows))};
	if (!p2) out_of_memory();

	if (test_part_iter_box(rows, cols)) out_of_memory();

	part_iter itr_p2;
	pitr_first(&itr_p2, p2.get(), rows, cols, nullptr, nullptr, 0, 0);
	for (; pitr_good(&itr_p2); pitr_next(&itr_p2))
	{
		uint32_t p2_len = iv_length(p2);
		part_unchop(p2.get(), rows);

		if (test_part_iter_sub(rows, cols, p2.get())) out_of_memory();
		if (test_part_iter_sub(rows0, cols, p2.get())) out_of_memory();
		if (test_part_iter_sub(rows, cols0, p2.get())) out_of_memory();
		if (test_part_iter_sub(rows, cols + 1, p2.get())) out_of_memory();
		if (test_part_iter_super(rows, cols, p2.get())) out_of_memory();
		if (test_part_iter_super(rows0, cols, p2.get())) out_of_memory();
		if (test_part_iter_super(rows, cols0, p2.get())) out_of_memory();
		if (test_part_iter_super(rows, cols + 2, p2.get())) out_of_memory();

		part_iter itr_p1;
		pitr_first(&itr_p1, p1.get(), rows, cols, p2.get(), nullptr, 0, PITR_USE_OUTER);
		for (; pitr_good(&itr_p1); pitr_next(&itr_p1))
		{
			uint32_t p1_len = iv_length(p1);
			part_unchop(p1.get(), rows);

			if (test_part_iter_between(rows, cols, p2.get(), p1.get())) out_of_memory();
			if (test_part_iter_between(rows0, cols, p2.get(), p1.get())) out_of_memory();
			if (test_part_iter_between(rows, cols0, p2.get(), p1.get())) out_of_memory();
			if (test_part_iter_between(rows, cols + 2, p2.get(), p1.get())) out_of_memory();

			iv_length(p1) = p1_len;
		}
		iv_length(p2) = p2_len;
	}

	puts("success");
	return 0;
}
