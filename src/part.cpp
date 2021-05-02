/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/part.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

int part_valid(const ivector* p)
{
	int x = 0;
	for (auto i = int(iv_length(p) - 1); i >= 0; i--)
	{
		int y = iv_elem(p, i);
		if (y < x) return 0;
		x = iv_elem(p, i);
	}
	return 1;
}

int part_decr(const ivector* p)
{
	for (uint32_t i = 1; i < iv_length(p); i++)
		if (iv_elem(p, i - 1) < iv_elem(p, i)) return 0;
	return 1;
}

uint32_t part_length(const ivector* p)
{
	uint32_t len = iv_length(p);
	while (len > 0 && iv_elem(p, len - 1) == 0) len--;
	return len;
}

int part_entry(const ivector* p, int i) { return (uint32_t(i) < iv_length(p)) ? iv_elem(p, i) : 0; }

/* Must have len >= iv_length(p) and p was allocated with enough space. */
void part_unchop(ivector* p, int len_)
{
	uint32_t len0 = iv_length(p);
	assert(int(len0) <= len_);
	auto len = uint32_t(len_);
	iv_length(p) = len;
	memset(p->array + len0, 0, (len - len0) * sizeof(p->array[0]));
}

int part_leq(const ivector* p1, const ivector* p2)
{
	uint32_t len = part_length(p1);
	if (len > part_length(p2)) return 0;
	for (auto i = int(len - 1); i >= 0; i--)
		if (iv_elem(p1, i) > iv_elem(p2, i)) return 0;
	return 1;
}

void part_print(const ivector* p)
{
	putchar('(');
	for (uint32_t i = 0; i < iv_length(p) && iv_elem(p, i) != 0; i++)
	{
		if (i) putchar(',');
		printf("%d", iv_elem(p, i));
	}
	putchar(')');
}

void part_printnl(const ivector* p)
{
	part_print(p);
	putchar('\n');
}

void part_print_lincomb(const ivlincomb* lc)
{
	for (const auto& kv : ivlc_iterator(lc))
	{
		if (kv.value == 0) continue;
		printf("%d  ", kv.value);
		part_printnl(kv.key);
	}
}

int part_qdegree(const ivector* p, int level)
{
	int n = int(iv_length(p)) + level;
	int d = 0;
	for (uint32_t i = 0; i < iv_length(p); i++)
	{
		int a = iv_elem(p, i) + int(iv_length(p)) - int(i) - 1;
		int b = (a >= 0) ? (a / n) : -((n - 1 - a) / n);
		d += b;
	}
	return d;
}

int part_qentry(const ivector* p, int i, int d, int level)
{
	int rows = int(iv_length(p));
	int k = (i + d) % rows;
	return iv_elem(p, k) - ((i + d) / rows) * level - d;
}

void part_qprint(const ivector* p, int level)
{
	int d = part_qdegree(p, level);
	putchar('(');
	for (uint32_t i = 0; i < iv_length(p); i++)
	{
		int x = part_qentry(p, int(i), d, level);
		if (x == 0) break;
		if (i) putchar(',');
		printf("%d", x);
	}
	putchar(')');
}

void part_qprintnl(const ivector* p, int level)
{
	part_qprint(p, level);
	putchar('\n');
}

void part_qprint_lincomb(const ivlincomb* lc, int level)
{
	for (const auto& kv : ivlc_iterator(lc))
	{
		if (kv.value == 0) continue;
		printf("%d  ", kv.value);
		part_qprintnl(kv.key, level);
	}
}

int pitr_good(const part_iter* itr) { return itr->rows >= 0; }

int pitr_first(part_iter* itr, ivector* p, int rows, int cols, const ivector* outer, const ivector* inner, int size,
               int opt)
{
	int use_outer = opt & PITR_USE_OUTER;
	int use_inner = opt & PITR_USE_INNER;
	int use_size = opt & PITR_USE_SIZE;

	assert((!use_outer) || part_valid(outer));
	assert((!use_inner) || part_valid(inner));
	assert((!use_outer) || (!use_inner) || part_leq(inner, outer));

	itr->part = p;
	if (use_outer) itr->outer = outer;
	if (use_inner) itr->inner = inner;
	itr->opt = opt;

	if (cols == 0) rows = 0;
	if (use_size && rows > size) rows = size;
	if (use_outer)
	{
		if (uint32_t(rows) > iv_length(outer)) rows = int(iv_length(outer));
		while (rows > 0 && iv_elem(outer, rows - 1) == 0) rows--;
	}
	itr->rows = rows;
	itr->length = rows;
	iv_set_zero(p);

	int inner_sz = 0;
	if (use_inner)
	{
		assert(iv_length(inner) >= uint32_t(rows));
		if (iv_length(inner) > uint32_t(rows) && iv_elem(inner, rows) != 0) goto empty_result;
		if (rows > 0 && cols < iv_elem(inner, 0)) goto empty_result;
	}

	if (use_size)
	{
		if (size > rows * cols) goto empty_result;
		if (use_inner)
		{
			inner_sz = iv_sum(inner);
			if (size < inner_sz) goto empty_result;
		}
	}

	int r;
	for (r = 0; r < rows; r++)
	{
		int c = cols;
		if (use_outer && c > iv_elem(outer, r)) c = iv_elem(outer, r);
		if (use_size)
		{
			int avail = size;
			if (use_inner)
			{
				inner_sz -= iv_elem(inner, r);
				avail -= inner_sz;
			}
			if (avail == 0)
			{
				itr->length = r;
				return 0;
			}
			if (c > avail) c = avail;
			size -= c;
		}
		iv_elem(p, r) = c;
	}

	if (use_size && size > 0) goto empty_result;

	itr->length = r;
	return 0;

empty_result:
	itr->rows = -1;
	return 0;
}

void pitr_box_first(part_iter* itr, ivector* p, int rows, int cols)
{
	pitr_first(itr, p, rows, cols, nullptr, nullptr, 0, 0);
}

void pitr_box_sz_first(part_iter* itr, ivector* p, int rows, int cols, int size)
{
	pitr_first(itr, p, rows, cols, nullptr, nullptr, size, PITR_USE_SIZE);
}

void pitr_next(part_iter* itr)
{
	ivector* p = itr->part;
	const ivector* outer = itr->outer;
	const ivector* inner = itr->inner;
	int rows = itr->rows;
	int opt = itr->opt;

	int use_outer = opt & PITR_USE_OUTER;
	int use_inner = opt & PITR_USE_INNER;
	int use_size = opt & PITR_USE_SIZE;

	int outer_row = rows;
	int size = 0, inner_sz = 0, outer_sz = 0;
	if (use_size)
	{
		size = 0;
		inner_sz = 0; /* number of boxes in inner[r..]. */
		outer_sz = 0; /* number of boxes in outer[outer_row..] */
	}

	for (int r = itr->length - 1; r >= 0; r--)
	{
		if (use_size) size += iv_elem(p, r);
		if (use_size && use_inner) inner_sz += iv_elem(inner, r);

		int c = iv_elem(p, r) - 1;

		if (use_inner && c < iv_elem(inner, r)) continue;

		if (use_size && use_outer)
		{
			/* update outer_row and outer_sz. */
			while (outer_row > 0 && iv_elem(outer, outer_row - 1) < c)
			{
				outer_row -= 1;
				outer_sz += iv_elem(outer, outer_row);
			}
		}

		if (use_size && size > c * (outer_row - r) + outer_sz) continue;

		/* can decrease iv_elem(p, r). */
		if (c == 0)
		{
			iv_elem(p, r) = 0;
			itr->length = r;
			return;
		}

		itr->length = rows;
		for (; r < outer_row; r++)
		{
			if ((!use_size) && use_outer && c > iv_elem(outer, r)) break;
			if (use_size)
			{
				int avail = size;
				if (use_inner)
				{
					inner_sz -= iv_elem(inner, r);
					avail -= inner_sz;
				}
				if (avail == 0) break;
				if (c > avail) c = avail;
				size -= c;
			}
			iv_elem(p, r) = c;
		}
		if (use_outer)
			for (; r < rows; r++)
			{
				c = iv_elem(outer, r);
				if (use_size)
				{
					int avail = size;
					if (use_inner)
					{
						inner_sz -= iv_elem(inner, r);
						avail -= inner_sz;
					}
					if (avail == 0) break;
					if (c > avail) c = avail;
					size -= c;
				}
				iv_elem(p, r) = c;
			}
		for (int j = r; j < itr->length; j++) iv_elem(p, j) = 0;
		itr->length = r;
		return;
	}
	itr->rows = -1;
}
