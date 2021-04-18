/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdio.h>

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

#define PROGNAME "test_partiter"

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory\n");
	alloc_report();
	exit(1);
}

int test_part_iter_box(int rows, int cols)
{
	part_iter itr;
	ivector* p;
	int i, np, np1, size;

	p = iv_new(rows);
	if (!p) return -1;

	np = 1;
	for (i = 1; i <= rows; i++) np = np * (cols + i) / i;

	np1 = 0;
	pitr_box_first(&itr, p, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p));
		np1++;
	}
	assert(np1 == np);

	np1 = 0;
	for (size = 0; size < rows * cols + 2; size++)
	{
		pitr_box_sz_first(&itr, p, rows, cols, size);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p));
			assert(iv_sum(p) == size);
			np1++;
		}
	}
	assert(np1 == np);
	iv_free(p);
	return 0;
}

int test_part_iter_sub(int rows, int cols, ivector* outer)
{
	part_iter itr;
	ivector *p, *count;
	int size_bound, size, np;

	size_bound = iv_sum(outer) + 2;
	count = iv_new_zero(size_bound);
	if (!count) return -1;

	p = iv_new(rows);
	if (!p)
	{
		iv_free(count);
		return -1;
	}

	pitr_box_first(&itr, p, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(p, outer))
		{
			int sz = iv_sum(p);
			iv_elem(count, sz)++;
		}

	np = 0;
	pitr_first(&itr, p, rows, cols, outer, NULL, 0, PITR_USE_OUTER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p));
		assert(part_leq(p, outer));
		np++;
	}
	assert(np == iv_sum(count));

	for (size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p, rows, cols, outer, NULL, size, PITR_USE_OUTER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p));
			assert(part_leq(p, outer));
			assert(iv_sum(p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	iv_free(p);
	iv_free(count);
	return 0;
}

int test_part_iter_super(int rows, int cols, ivector* inner)
{
	part_iter itr;
	ivector *p, *count;
	int size_bound, size, np;

	size_bound = rows * cols + 2;
	count = iv_new_zero(size_bound);
	if (!count) return -1;

	p = iv_new(rows);
	if (!p)
	{
		iv_free(count);
		return -1;
	}

	pitr_box_first(&itr, p, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(inner, p))
		{
			int sz = iv_sum(p);
			iv_elem(count, sz)++;
		}

	np = 0;
	pitr_first(&itr, p, rows, cols, NULL, inner, 0, PITR_USE_INNER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p));
		assert(part_leq(inner, p));
		np++;
	}
	assert(np == iv_sum(count));

	for (size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p, rows, cols, NULL, inner, size, PITR_USE_INNER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p));
			assert(part_leq(inner, p));
			assert(iv_sum(p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	iv_free(p);
	iv_free(count);
	return 0;
}

int test_part_iter_between(int rows, int cols, ivector* outer, ivector* inner)
{
	part_iter itr;
	ivector *p, *count;
	int size_bound, size, np;

	size_bound = iv_sum(outer) + 2;
	count = iv_new_zero(size_bound);
	if (!count) return -1;

	p = iv_new(rows);
	if (!p)
	{
		iv_free(count);
		return -1;
	}

	pitr_box_first(&itr, p, rows, cols);
	for (; pitr_good(&itr); pitr_next(&itr))
		if (part_leq(inner, p) && part_leq(p, outer))
		{
			int sz = iv_sum(p);
			iv_elem(count, sz)++;
		}

	np = 0;
	pitr_first(&itr, p, rows, cols, outer, inner, 0, PITR_USE_OUTER | PITR_USE_INNER);
	for (; pitr_good(&itr); pitr_next(&itr))
	{
		assert(part_valid(p));
		assert(part_leq(inner, p));
		assert(part_leq(p, outer));
		np++;
	}
	assert(np == iv_sum(count));

	for (size = 0; size < size_bound; size++)
	{
		np = 0;
		pitr_first(&itr, p, rows, cols, outer, inner, size, PITR_USE_OUTER | PITR_USE_INNER | PITR_USE_SIZE);
		for (; pitr_good(&itr); pitr_next(&itr))
		{
			assert(part_valid(p));
			assert(part_leq(inner, p));
			assert(part_leq(p, outer));
			assert(iv_sum(p) == size);
			np++;
		}
		assert(np == iv_elem(count, size));
	}

	iv_free(p);
	iv_free(count);
	return 0;
}

int main(int ac, char** av)
{
	int rows, cols, rows0, cols0;
	part_iter itr_p1, itr_p2;
	ivector *p1, *p2;

	alloc_getenv();

	if (ac != 3)
	{
		fprintf(stderr, "usage: " PROGNAME " rows cols\n");
		exit(1);
	}
	rows = atol(av[1]);
	cols = atol(av[2]);
	rows0 = rows ? rows - 1 : 0;
	cols0 = cols ? cols - 1 : 0;

	p1 = p2 = NULL;

	p1 = iv_new(rows);
	if (!p1) goto out_of_mem;
	p2 = iv_new(rows);
	if (!p2) goto out_of_mem;

	if (test_part_iter_box(rows, cols)) goto out_of_mem;

	pitr_first(&itr_p2, p2, rows, cols, NULL, NULL, 0, 0);
	for (; pitr_good(&itr_p2); pitr_next(&itr_p2))
	{
		int p2_len = iv_length(p2);
		part_unchop(p2, rows);

		if (test_part_iter_sub(rows, cols, p2)) goto out_of_mem;
		if (test_part_iter_sub(rows0, cols, p2)) goto out_of_mem;
		if (test_part_iter_sub(rows, cols0, p2)) goto out_of_mem;
		if (test_part_iter_sub(rows, cols + 1, p2)) goto out_of_mem;
		if (test_part_iter_super(rows, cols, p2)) goto out_of_mem;
		if (test_part_iter_super(rows0, cols, p2)) goto out_of_mem;
		if (test_part_iter_super(rows, cols0, p2)) goto out_of_mem;
		if (test_part_iter_super(rows, cols + 2, p2)) goto out_of_mem;

		pitr_first(&itr_p1, p1, rows, cols, p2, NULL, 0, PITR_USE_OUTER);
		for (; pitr_good(&itr_p1); pitr_next(&itr_p1))
		{
			int p1_len = iv_length(p1);
			part_unchop(p1, rows);

			if (test_part_iter_between(rows, cols, p2, p1)) goto out_of_mem;
			if (test_part_iter_between(rows0, cols, p2, p1)) goto out_of_mem;
			if (test_part_iter_between(rows, cols0, p2, p1)) goto out_of_mem;
			if (test_part_iter_between(rows, cols + 2, p2, p1)) goto out_of_mem;

			iv_length(p1) = p1_len;
		}
		iv_length(p2) = p2_len;
	}
	iv_free(p1);
	iv_free(p2);

	puts("success");
	alloc_report();
	return 0;

out_of_mem:
	if (p1) iv_free(p1);
	if (p2) iv_free(p2);
	out_of_memory();
}
