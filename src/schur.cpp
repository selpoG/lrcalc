/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/alloc.hpp"
#include "lrcalc/ilist.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/lrcoef.hpp"
#include "lrcalc/lriter.hpp"
#include "lrcalc/optshape.hpp"
#include "lrcalc/part.hpp"

#include "lrcalc/schur.hpp"

#include <memory>

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
struct il_deleter
{
	void operator()(ilist* p) const { il_free(p); }
};
struct ivl_deleter
{
	void operator()(ivlist* p) const { ivl_free_all(p); }
};
using safe_iv_ptr = std::unique_ptr<ivector, iv_deleter>;
using safe_il_ptr = std::unique_ptr<ilist, il_deleter>;
using safe_ivl_ptr = std::unique_ptr<ivlist, ivl_deleter>;

ivlincomb* schur_mult(ivector* sh1, ivector* sh2, int rows, int cols, int partsz)
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

int fusion_reduce(ivector* la, int level, ivector* tmp)
{
	claim(iv_length(la) == iv_length(tmp));
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

bool fusion_reduce_lc(ivlincomb* lc, int level)
{
	/* Copy linear combination to lists. */
	safe_ivl_ptr parts{ivl_new(ivlc_card(lc))};
	if (!parts) return true;
	safe_il_ptr coefs{il_new(ivlc_card(lc))};
	if (!coefs) return true;

	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivl_append(parts.get(), ivlc_key(&itr));
		il_append(coefs.get(), ivlc_value(&itr));
	}
	ivlc_reset(lc);

	safe_iv_ptr tmp;
	if (ivl_length(parts) > 0)
	{
		ivector* sh = ivl_elem(parts, 0);
		tmp.reset(iv_new(iv_length(sh)));
		if (!tmp) return true;
	}

	/* Reduce and reinsert terms. */
	while (ivl_length(parts) != 0)
	{
		ivector* sh = ivl_poplast(parts.get());
		int c = il_poplast(coefs.get());
		int sign = fusion_reduce(sh, level, tmp.get());
		if (ivlc_add_element(lc, sign * c, sh, iv_hash(sh), LC_FREE_KEY | LC_FREE_ZERO) != 0) return true;
	}

	return false;
}

ivlincomb* schur_mult_fusion(ivector* sh1, ivector* sh2, int rows, int level)
{
	claim(part_valid(sh1) && part_valid(sh2));
	if (part_entry(sh1, rows) != 0 || part_entry(sh2, rows) != 0) return ivlc_new(5, 2);

	int sign = 1;
	safe_iv_ptr tmp, nsh1, nsh2;
	if (part_entry(sh1, 0) - part_entry(sh1, rows - 1) > level)
	{
		tmp.reset(iv_new(uint32_t(rows)));
		if (!tmp) return nullptr;
		nsh1.reset(iv_new(uint32_t(rows)));
		if (!nsh1) return nullptr;
		for (int i = 0; i < rows; i++) iv_elem(nsh1, i) = part_entry(sh1, i);
		sh1 = nsh1.get();
		sign = fusion_reduce(sh1, level, tmp.get());
	}
	if (sign == 0) return ivlc_new(5, 2);
	if (part_entry(sh2, 0) - part_entry(sh2, rows - 1) > level)
	{
		if (!tmp) tmp.reset(iv_new(uint32_t(rows)));
		if (!tmp) return nullptr;
		nsh2.reset(iv_new(uint32_t(rows)));
		if (!nsh2) return nullptr;
		for (int i = 0; i < rows; i++) iv_elem(nsh2, i) = part_entry(sh2, i);
		sh2 = nsh2.get();
		sign *= fusion_reduce(sh2, level, tmp.get());
	}
	if (sign == 0) return ivlc_new(5, 2);

	skew_shape ss;
	if (optim_fusion(&ss, sh1, sh2, rows, level) != 0) return nullptr;
	ivlincomb* lc;
	if (ss.sign)
		lc = lrit_expand(ss.outer, nullptr, ss.cont, rows, -1, rows);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	if (lc == nullptr) return nullptr;

	if (fusion_reduce_lc(lc, level))
	{
		ivlc_free_all(lc);
		return nullptr;
	}

	if (sign < 0)
	{
		ivlc_iter itr;
		for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
		{
			ivlc_keyval_t* kv = ivlc_keyval(&itr);
			kv->value = -kv->value;
		}
	}

	return lc;
}

ivlincomb* schur_skew(ivector* outer, ivector* inner, int rows, int partsz)
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

static inline int _schur_coprod_isredundant(ivector* cont, int rows, int cols)
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

static inline ivlincomb* _schur_coprod_count(lrtab_iter* lrit, int rows, int cols)
{
	ivector* cont = lrit->cont;
	ivlincomb* lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (lc == nullptr) return nullptr;
	for (; lrit_good(lrit); lrit_next(lrit))
	{
		if (_schur_coprod_isredundant(cont, rows, cols)) continue;
		if (ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY) != 0)
		{
			ivlc_free_all(lc);
			return nullptr;
		}
	}
	return lc;
}

static ivlincomb* _schur_coprod_expand(ivector* outer, ivector* content, int rows, int cols, int partsz)
{
	lrtab_iter* lrit = lrit_new(outer, nullptr, content, -1, -1, partsz);
	if (lrit == nullptr) return nullptr;
	ivlincomb* lc = _schur_coprod_count(lrit, rows, cols);
	lrit_free(lrit);
	return lc;
}

ivlincomb* schur_coprod(ivector* sh, int rows, int cols, int partsz, int all)
{
	safe_iv_ptr box{iv_new(uint32_t(rows))};
	if (!box) return nullptr;
	for (int i = 0; i < rows; i++) iv_elem(box, i) = cols;

	if (all) return schur_mult(sh, box.get(), -1, -1, partsz);

	skew_shape ss;
	if (optim_mult(&ss, sh, box.get(), -1, -1) != 0) return nullptr;

	ivlincomb* lc = _schur_coprod_expand(ss.outer, ss.cont, rows, cols, partsz);
	sksh_dealloc(&ss);
	return lc;
}

long long schur_lrcoef(ivector* outer, ivector* inner1, ivector* inner2)
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
