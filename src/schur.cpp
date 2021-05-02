/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/schur.hpp"

#include <assert.h>
#include <stdint.h>

#include <vector>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/lrcoef.hpp"
#include "lrcalc/lriter.hpp"
#include "lrcalc/optshape.hpp"
#include "lrcalc/part.hpp"

ivlincomb* schur_mult(const ivector* sh1, const ivector* sh2, int rows, int cols, int partsz)
{
	skew_shape ss;
	if (optim_mult(&ss, sh1, sh2, rows, cols) != 0) return nullptr;
	ivlincomb* lc;
	if (ss.sign)
		lc = lrit_expand(ss.outer, nullptr, ss.cont, rows, cols, partsz);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	return lc;
}

static int fusion_reduce(ivector* la, int level, iv_ptr& tmp)
{
	assert(iv_length(la) == iv_length(tmp));
	int rows = int(iv_length(la));
	int n = rows + level;

	int q = 0;
	for (int i = 0; i < rows; i++)
	{
		int a = iv_elem(la, i) + rows - i - 1;
		int b = (a >= 0) ? (a / n) : -((n - 1 - a) / n);
		q += b;
		iv_elem(tmp, i) = a - b * n - rows + 1;
	}

	/* bubble sort */
	int sign = (rows & 1) ? 0 : q;
	for (int i = 0; i < rows - 1; i++)
	{
		int k = i;
		int a = iv_elem(tmp, k);
		for (int j = i + 1; j < rows; j++)
			if (a < iv_elem(tmp, j))
			{
				k = j;
				a = iv_elem(tmp, k);
			}
		if (k != i)
		{
			iv_elem(tmp, k) = iv_elem(tmp, i);
			iv_elem(tmp, i) = a;
			sign++;
		}
	}

	for (int i = 0; i < rows; i++)
	{
		if (i > 0 && iv_elem(tmp, i - 1) == iv_elem(tmp, i)) return 0;
		int k = i + q;
		int a = iv_elem(tmp, i) + k + (k / rows) * level;
		iv_elem(la, (k + rows) % rows) = a;
	}

	return (sign & 1) ? -1 : 1;
}

int fusion_reduce_lc(ivlincomb* lc, int level)
{
	/* Copy linear combination to lists. */
	ivl_ptr parts{ivl_new(ivlc_card(lc))};
	if (!parts) return 1;
	try
	{
		std::vector<int> coefs;
		coefs.reserve(ivlc_card(lc));

		for (auto& kv : ivlc_iterator(lc))
		{
			ivl_append(parts.get(), kv.key);
			coefs.push_back(kv.value);
		}
		ivlc_reset(lc);

		iv_ptr tmp;
		if (ivl_length(parts) > 0)
		{
			const ivector* sh = ivl_elem(parts, 0);
			tmp = iv_create(iv_length(sh));
			if (!tmp) return 1;
		}

		/* Reduce and reinsert terms. */
		while (ivl_length(parts) != 0)
		{
			ivector* sh = ivl_poplast(parts.get());
			int c = coefs.back();
			coefs.pop_back();
			int sign = fusion_reduce(sh, level, tmp);
			if (ivlc_add_element(lc, sign * c, sh, iv_hash(sh), LC_FREE_KEY | LC_FREE_ZERO) != 0) return true;
		}
	}
	catch (const std::bad_alloc&)
	{
		return 1;
	}

	return 0;
}

ivlincomb* schur_mult_fusion(const ivector* sh1, const ivector* sh2, int rows, int level)
{
	assert(part_valid(sh1) && part_valid(sh2));
	if (part_entry(sh1, rows) != 0 || part_entry(sh2, rows) != 0) return ivlc_new(5, 2);

	int sign = 1;
	iv_ptr tmp, nsh1, nsh2;
	if (part_entry(sh1, 0) - part_entry(sh1, rows - 1) > level)
	{
		tmp = iv_create(uint32_t(rows));
		if (!tmp) return nullptr;
		nsh1 = iv_create(uint32_t(rows));
		if (!nsh1) return nullptr;
		for (int i = 0; i < rows; i++) iv_elem(nsh1, i) = part_entry(sh1, i);
		sign = fusion_reduce(nsh1.get(), level, tmp);
		sh1 = nsh1.get();
	}
	if (sign == 0) return ivlc_new(5, 2);
	if (part_entry(sh2, 0) - part_entry(sh2, rows - 1) > level)
	{
		if (!tmp) tmp = iv_create(uint32_t(rows));
		if (!tmp) return nullptr;
		nsh2 = iv_create(uint32_t(rows));
		if (!nsh2) return nullptr;
		for (int i = 0; i < rows; i++) iv_elem(nsh2, i) = part_entry(sh2, i);
		sign *= fusion_reduce(nsh2.get(), level, tmp);
		sh2 = nsh2.get();
	}
	if (sign == 0) return ivlc_new(5, 2);

	skew_shape ss;
	if (optim_fusion(&ss, sh1, sh2, rows, level) != 0) return nullptr;
	ivlc_ptr lc;
	if (ss.sign)
		lc.reset(lrit_expand(ss.outer, nullptr, ss.cont, rows, -1, rows));
	else
		lc.reset(ivlc_new(5, 2));
	sksh_dealloc(&ss);
	if (!lc) return nullptr;

	if (fusion_reduce_lc(lc.get(), level)) return nullptr;

	if (sign < 0)
		for (auto& kv : ivlc_iterator(lc)) kv.value = -kv.value;

	return lc.release();
}

ivlincomb* schur_skew(const ivector* outer, const ivector* inner, int rows, int partsz)
{
	skew_shape ss;
	if (optim_skew(&ss, outer, inner, nullptr, rows) != 0) return nullptr;
	ivlincomb* lc;
	if (ss.sign)
		lc = lrit_expand(ss.outer, ss.inner, ss.cont, rows, -1, partsz);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	return lc;
}

static int _schur_coprod_isredundant(const ivector* cont, int rows, int cols)
{
	int sz1 = -rows * cols;
	for (int i = 0; i < rows; i++) sz1 += iv_elem(cont, i);
	int sz2 = 0;
	for (int i = rows; i < int(iv_length(cont)); i++) sz2 += iv_elem(cont, i);
	if (sz1 != sz2) return (sz1 > sz2) ? 0 : 1;
	for (int i = 0; i < rows; i++)
	{
		int df = iv_elem(cont, i) - cols - part_entry(cont, rows + i);
		if (df) return (df < 0) ? 0 : 1;
	}
	return 0;
}

static ivlc_ptr _schur_coprod_count(lrtab_iter* lrit, int rows, int cols)
{
	ivector* cont = lrit->cont;
	ivlc_ptr lc = ivlc_create();
	if (!lc) return {};
	for (; lrit_good(lrit); lrit_next(lrit))
	{
		if (_schur_coprod_isredundant(cont, rows, cols)) continue;
		if (ivlc_add_element(lc.get(), 1, cont, iv_hash(cont), LC_COPY_KEY) != 0) return {};
	}
	return lc;
}

static ivlc_ptr _schur_coprod_expand(const ivector* outer, const ivector* content, int rows, int cols, int partsz)
{
	lrtab_iter* lrit = lrit_new(outer, nullptr, content, -1, -1, partsz);
	if (lrit == nullptr) return {};
	ivlc_ptr lc = _schur_coprod_count(lrit, rows, cols);
	lrit_free(lrit);
	return lc;
}

ivlincomb* schur_coprod(const ivector* sh, int rows, int cols, int partsz, int all)
{
	iv_ptr box = iv_create(uint32_t(rows));
	if (!box) return nullptr;
	for (int i = 0; i < rows; i++) iv_elem(box, i) = cols;

	if (all) return schur_mult(sh, box.get(), -1, -1, partsz);

	skew_shape ss;
	if (optim_mult(&ss, sh, box.get(), -1, -1) != 0) return nullptr;

	ivlc_ptr lc = _schur_coprod_expand(ss.outer, ss.cont, rows, cols, partsz);
	sksh_dealloc(&ss);
	return lc.release();
}

long long schur_lrcoef(const ivector* outer, const ivector* inner1, const ivector* inner2)
{
	skew_shape ss;
	if (optim_coef(&ss, outer, inner1, inner2) != 0) return -1;
	long long coef;
	if (ss.sign <= 1)
		coef = ss.sign;
	else
		coef = lrcoef_count(ss.outer, ss.inner, ss.cont);
	sksh_dealloc(&ss);
	return coef;
}
