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

#define _SCHUR_C
#include "lrcalc/schur.hpp"

ivlincomb* schur_mult(ivector* sh1, ivector* sh2, int rows, int cols, int partsz)
{
	skew_shape ss;
	ivlincomb* lc;
	if (optim_mult(&ss, sh1, sh2, rows, cols) != 0) return NULL;
	if (ss.sign)
		lc = lrit_expand(ss.outer, NULL, ss.cont, rows, cols, partsz);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	return lc;
}

int fusion_reduce(ivector* la, int level, ivector* tmp)
{
	int rows, n, q, i, j, k, a, b, sign;

	claim(iv_length(la) == iv_length(tmp));
	rows = iv_length(la);
	n = rows + level;

	q = 0;
	for (i = 0; i < rows; i++)
	{
		a = iv_elem(la, i) + rows - i - 1;
		b = (a >= 0) ? (a / n) : -((n - 1 - a) / n);
		q += b;
		iv_elem(tmp, i) = a - b * n - rows + 1;
	}

	/* bubble sort */
	sign = (rows & 1) ? 0 : q;
	for (i = 0; i < rows - 1; i++)
	{
		k = i;
		a = iv_elem(tmp, k);
		for (j = i + 1; j < rows; j++)
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

	for (i = 0; i < rows; i++)
	{
		if (i > 0 && iv_elem(tmp, i - 1) == iv_elem(tmp, i)) return 0;
		k = i + q;
		a = iv_elem(tmp, i) + k + (k / rows) * level;
		iv_elem(la, (k + rows) % rows) = a;
	}

	return (sign & 1) ? -1 : 1;
}

int fusion_reduce_lc(ivlincomb* lc, int level)
{
	ivlc_iter itr;
	ivlist* parts;
	ilist* coefs;
	ivector *sh, *tmp;
	int c, sign, ok;

	parts = NULL;
	coefs = NULL;
	tmp = NULL;
	ok = -1;

	/* Copy linear combination to lists. */
	parts = ivl_new(ivlc_card(lc));
	if (parts == NULL) goto free_return;
	coefs = il_new(ivlc_card(lc));
	if (coefs == NULL) goto free_return;

	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivl_append(parts, ivlc_key(&itr));
		il_append(coefs, ivlc_value(&itr));
	}
	ivlc_reset(lc);

	if (ivl_length(parts) > 0)
	{
		sh = ivl_elem(parts, 0);
		tmp = iv_new(iv_length(sh));
		if (tmp == NULL) goto free_return;
	}

	/* Reduce and reinsert terms. */
	while (ivl_length(parts) != 0)
	{
		sh = ivl_poplast(parts);
		c = il_poplast(coefs);
		sign = fusion_reduce(sh, level, tmp);
		if (ivlc_add_element(lc, sign * c, sh, iv_hash(sh), LC_FREE_KEY | LC_FREE_ZERO) != 0) goto free_return;
	}
	ok = 0;

free_return:
	if (tmp != NULL) iv_free(tmp);
	if (coefs != NULL) il_free(coefs);
	if (parts != NULL) ivl_free_all(parts);
	return ok;
}

ivlincomb* schur_mult_fusion(ivector* sh1, ivector* sh2, int rows, int level)
{
	skew_shape ss;
	ivlincomb* lc;
	ivector *nsh1, *nsh2, *tmp;
	int sign, i;

	claim(part_valid(sh1) && part_valid(sh2));
	if (part_entry(sh1, rows) != 0 || part_entry(sh2, rows) != 0) return ivlc_new(5, 2);

	nsh1 = nsh2 = tmp = NULL;
	lc = NULL;

	sign = 1;
	if (part_entry(sh1, 0) - part_entry(sh1, rows - 1) > level)
	{
		tmp = iv_new(rows);
		if (tmp == NULL) goto free_return;
		nsh1 = iv_new(rows);
		if (nsh1 == NULL) goto free_return;
		for (i = 0; i < rows; i++) iv_elem(nsh1, i) = part_entry(sh1, i);
		sh1 = nsh1;
		sign = fusion_reduce(sh1, level, tmp);
	}
	if (sign == 0)
	{
		lc = ivlc_new(5, 2);
		goto free_return;
	}
	if (part_entry(sh2, 0) - part_entry(sh2, rows - 1) > level)
	{
		if (tmp == NULL) tmp = iv_new(rows);
		if (tmp == NULL) goto free_return;
		nsh2 = iv_new(rows);
		if (nsh2 == NULL) goto free_return;
		for (i = 0; i < rows; i++) iv_elem(nsh2, i) = part_entry(sh2, i);
		sh2 = nsh2;
		sign *= fusion_reduce(sh2, level, tmp);
	}
	if (sign == 0)
	{
		lc = ivlc_new(5, 2);
		goto free_return;
	}

	if (optim_fusion(&ss, sh1, sh2, rows, level) != 0) goto free_return;
	if (ss.sign)
		lc = lrit_expand(ss.outer, NULL, ss.cont, rows, -1, rows);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	if (lc == NULL) goto free_return;

	if (fusion_reduce_lc(lc, level) != 0)
	{
		ivlc_free_all(lc);
		lc = NULL;
		goto free_return;
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

free_return:
	if (tmp) iv_free(tmp);
	if (nsh1) iv_free(nsh1);
	if (nsh2) iv_free(nsh2);
	return lc;
}

ivlincomb* schur_skew(ivector* outer, ivector* inner, int rows, int partsz)
{
	skew_shape ss;
	ivlincomb* lc;
	if (optim_skew(&ss, outer, inner, NULL, rows) != 0) return NULL;
	if (ss.sign)
		lc = lrit_expand(ss.outer, ss.inner, ss.cont, rows, -1, partsz);
	else
		lc = ivlc_new(5, 2);
	sksh_dealloc(&ss);
	return lc;
}

static inline int _schur_coprod_isredundant(ivector* cont, int rows, int cols)
{
	int i, sz1, sz2;
	sz1 = -rows * cols;
	for (i = 0; i < rows; i++) sz1 += iv_elem(cont, i);
	sz2 = 0;
	for (i = rows; i < iv_length(cont); i++) sz2 += iv_elem(cont, i);
	if (sz1 != sz2) return (sz1 > sz2) ? 0 : 1;
	for (i = 0; i < rows; i++)
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
	if (lc == NULL) return NULL;
	for (; lrit_good(lrit); lrit_next(lrit))
	{
		if (_schur_coprod_isredundant(cont, rows, cols)) continue;
		if (ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY) != 0)
		{
			ivlc_free_all(lc);
			return NULL;
		}
	}
	return lc;
}

static ivlincomb* _schur_coprod_expand(ivector* outer, ivector* content, int rows, int cols, int partsz)
{
	lrtab_iter* lrit;
	ivlincomb* lc;
	lrit = lrit_new(outer, NULL, content, -1, -1, partsz);
	if (lrit == NULL) return NULL;
	lc = _schur_coprod_count(lrit, rows, cols);
	lrit_free(lrit);
	return lc;
}

ivlincomb* schur_coprod(ivector* sh, int rows, int cols, int partsz, int all)
{
	skew_shape ss;
	ivlincomb* lc;
	ivector* box;
	int i;

	box = iv_new(rows);
	if (box == NULL) return NULL;
	for (i = 0; i < rows; i++) iv_elem(box, i) = cols;

	if (all)
	{
		lc = schur_mult(sh, box, -1, -1, partsz);
		iv_free(box);
		return lc;
	}

	if (optim_mult(&ss, sh, box, -1, -1) != 0)
	{
		iv_free(box);
		return NULL;
	}

	lc = _schur_coprod_expand(ss.outer, ss.cont, rows, cols, partsz);
	sksh_dealloc(&ss);
	iv_free(box);
	return lc;
}

long long schur_lrcoef(ivector* outer, ivector* inner1, ivector* inner2)
{
	skew_shape ss;
	long long coef;
	if (optim_coef(&ss, outer, inner1, inner2) != 0) return -1;
	if (ss.sign <= 1)
		coef = ss.sign;
	else
		coef = lrcoef_count(ss.outer, ss.inner, ss.cont);
	sksh_dealloc(&ss);
	return coef;
}
