/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/lriter.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"

lrtab_iter* lrit_new(const ivector* outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
                     int partsz)
{
	assert(part_valid(outer));
	assert(inner == nullptr || part_valid(inner));
	assert(content == nullptr || part_decr(content));

	/* Empty result if inner not contained in outer. */
	if (inner != nullptr && !part_leq(inner, outer))
	{
		iv_ptr cont = iv_create(1);
		auto lrit = new (std::nothrow) lrtab_iter;
		if (lrit == nullptr) return nullptr;
		lrit->cont = cont.release();
		lrit->size = -1;
		return lrit;
	}

	uint32_t len = part_length(outer);
	uint32_t ilen = (inner == nullptr) ? 0 : iv_length(inner);
	if (ilen > len) ilen = len;
	uint32_t clen = (content == nullptr) ? 0 : part_length(content);
	int out0 = (len == 0) ? 0 : iv_elem(outer, 0);
	assert(maxcols < 0 || ilen == 0 || iv_elem(inner, 0) == 0);

	/* Find number of boxes and maximal tableau entry. */
	int size = 0;
	auto maxdepth = int(clen);
	for (uint32_t r = 0; r < len; r++)
	{
		int inn_r = (r < ilen) ? iv_elem(inner, r) : 0;
		int rowsz = iv_elem(outer, r) - inn_r;
		size += rowsz;
		if (rowsz > 0) maxdepth++;
	}
	if (maxrows < 0 || maxrows > maxdepth) maxrows = maxdepth;

	/* Find size of array. */
	int array_len = size + 2;
	if (maxcols >= 0)
	{
		int clim = maxcols - out0;
		int c1 = 0;
		for (int r = int(clen - 1); r >= 0; r--)
		{
			int c0 = iv_elem(content, r);
			if (c1 < c0 && c1 < maxcols && c0 > clim) array_len++;
			c1 = c0;
		}
		if (c1 >= maxcols) array_len--;
	}

	/* Allocate array. */
	auto arr = new (std::nothrow) lrit_box[uint32_t(array_len)];
	if (arr == nullptr) return nullptr;
	auto lrit = new (std::nothrow) lrtab_iter;
	if (lrit == nullptr)
	{
		delete[] arr;
		return nullptr;
	}
	lrit->array_len = array_len;
	lrit->array = arr;

	/* Allocate and copy content. */
	if (partsz < maxrows) partsz = maxrows;
	auto partsz_u = uint32_t(partsz);
	ivector* cont = (lrit->cont = iv_new(partsz_u));
	lrit->size = -1;
	if (maxrows < int(clen)) return lrit; /* empty result. */
	{
		uint32_t r;
		for (r = 0; r < clen; r++) iv_elem(cont, r) = iv_elem(content, r);
		for (; r < partsz_u; r++) iv_elem(cont, r) = 0;
	}

	/* Check for empty result. */
	if (maxcols >= 0 && clen > 0 && iv_elem(cont, 0) > maxcols) return lrit; /* empty result. */
	if (maxcols >= 0 && out0 > maxcols) return lrit;                         /* empty result. */

	/* Initialize box structure. */
	{
		int s = 0;
		int out1 = 0;
		int inn0 = (len == 0) ? out0 : (len <= ilen ? iv_elem(inner, len - 1) : 0);
		for (auto r = int(len - 1); r >= 0; r--)
		{
			int out2 = out1;
			int inn1 = inn0;
			out1 = iv_elem(outer, r);
			inn0 = (r == 0) ? out0 : (r <= int(ilen) ? iv_elem(inner, r - 1) : 0);
			if (inn1 < out1) maxdepth--;
			for (int c = inn1; c < out1; c++)
			{
				lrit_box* box = lrit->array + s;
				int max;
				box->right = (c + 1 < out1) ? (s + 1) : (array_len - 1);
				box->above = (c >= inn0) ? (s + out1 - inn0) : size;
				max = (c < out2) ? (lrit->array[s - out2 + inn1].max - 1) : (maxrows - 1);
				box->max = (max < maxdepth) ? max : maxdepth;
				s++;
			}
		}
	}
	assert(maxdepth == int(clen));

	/* Set values of extra boxes. */
	lrit->array[array_len - 1].value = maxrows - 1;
	lrit->array[size].value = -1;
	if (maxcols >= 0)
	{
		int clim = maxcols - out0;
		int c1 = 0;
		int s = array_len - 2;
		int i = out0;
		for (auto r = int(clen - 1); r >= 0; r--)
		{
			int c0 = iv_elem(content, r);
			if (c1 < c0 && c1 < maxcols && c0 > clim)
			{
				lrit->array[s].value = r;
				while (i > maxcols - c0 && i > 0) lrit->array[size - out0 + (--i)].above = s;
				s--;
			}
			c1 = c0;
		}
	}

	/* Minimal LR tableau. */
	for (int s = size - 1; s >= 0; s--)
	{
		lrit_box* box = lrit->array + s;
		int x = lrit->array[box->above].value + 1;
		if (x > box->max) return lrit; /* empty result. */
		box->value = x;
		iv_elem(cont, x)++;
	}

	lrit->size = size;
	return lrit;
}

void lrit_free(lrtab_iter* lrit)
{
	iv_free(lrit->cont);
	delete[] lrit->array;
	delete lrit;
}

bool lrit_good(const lrtab_iter* lrit) { return lrit->size >= 0; }

void lrit_next(lrtab_iter* lrit)
{
	ivector* cont = lrit->cont;
	lrit_box* array = lrit->array;
	int size = lrit->size;
	lrit_box* box_bound = array + size;
	lrit_box* box;
	for (box = array; box != box_bound; box++)
	{
		int max = array[box->right].value;
		if (max > box->max) max = box->max;
		int x = box->value;
		iv_elem(cont, x)--;
		x++;
		while (x <= max && iv_elem(cont, x) == iv_elem(cont, x - 1)) x++;
		if (x > max) continue;

		/* Refill tableau with minimal values. */
		box->value = x;
		iv_elem(cont, x)++;
		while (box != array)
		{
			box--;
			x = array[box->above].value + 1;
			box->value = x;
			iv_elem(cont, x)++;
		}
		return;
	}
	lrit->size = -1;
}

static ivlincomb* lrit_count(lrtab_iter* lrit)
{
	ivector* cont = lrit->cont;
	ivlc_ptr lc = ivlc_create();
	for (; lrit_good(lrit); lrit_next(lrit)) ivlc_add_element(lc.get(), 1, cont, iv_hash(cont), LC_COPY_KEY);
	return lc.release();
}

ivlincomb* lrit_expand(const ivector* outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
                       int partsz)
{
	lrtab_iter* lrit = lrit_new(outer, inner, content, maxrows, maxcols, partsz);
	if (lrit == nullptr) return nullptr;
	ivlincomb* lc = lrit_count(lrit);
	lrit_free(lrit);
	return lc;
}
