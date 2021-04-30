/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/perm.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

int perm_valid(ivector* w)
{
	uint32_t n = iv_length(w);
	for (uint32_t i = 0; i < n; i++)
	{
		int a = abs(iv_elem(w, i)) - 1;
		if (a < 0 || a >= int(n) || iv_elem(w, a) < 0) return 0;
		iv_elem(w, a) = -iv_elem(w, a);
	}
	for (uint32_t i = 0; i < n; i++) iv_elem(w, i) = -iv_elem(w, i);
	return 1;
}

int perm_length(const ivector* w)
{
	uint32_t n = iv_length(w);
	int res = 0;
	for (uint32_t i = 0; i + 1 < n; i++)
		for (uint32_t j = i + 1; j < n; j++)
			if (iv_elem(w, i) > iv_elem(w, j)) res++;
	return res;
}

int perm_group(const ivector* w)
{
	int i = int(iv_length(w));
	while (i > 0 && iv_elem(w, i - 1) == i) i--;
	return i;
}

int dimvec_valid(const ivector* dv)
{
	uint32_t ld = iv_length(dv);
	if (ld == 0) return 0;
	if (iv_elem(dv, 0) < 0) return 0;
	for (uint32_t i = 1; i < ld; i++)
		if (iv_elem(dv, i - 1) > iv_elem(dv, i)) return 0;
	return 1;
}

/* Return 1 if S_w1 * S_w2 = 0 in H^*(Fl(rank)). */
int bruhat_zero(const ivector* w1, const ivector* w2, int rank)
{
	int n1 = perm_group(w1);
	int n2 = perm_group(w2);
	if (n1 > rank || n2 > rank) return 1;
	if (n1 > n2)
	{
		const ivector* tmp = w1;
		w1 = w2;
		w2 = tmp;
		n1 = n2;
	}
	for (int q = 1; q < n1; q++)
	{
		int q2 = rank - q;
		int r1 = 0;
		int r2 = 0;
		for (int p = 0; p < n1 - 1; p++)
		{
			if (iv_elem(w1, p) <= q) r1++;
			if (iv_elem(w2, p) > q2) r2++;
			if (r1 < r2) return 1;
		}
	}
	return 0;
}

ivlist* all_strings(const ivector* dimvec)
{
	assert(dimvec_valid(dimvec));

	uint32_t ld = iv_length(dimvec);
	ivector* cntvec = iv_new_zero(ld);
	if (cntvec == nullptr) return nullptr;
	int n_ = iv_elem(dimvec, ld - 1);
	if (n_ < 0)
	{
		iv_free(cntvec);
		return nullptr;
	}
	auto n = uint32_t(n_);

	ivlist* res = nullptr;
	ivector* str = iv_new(n);
	if (str == nullptr) goto out_of_memory;
	{
		uint32_t j = 0;
		for (uint32_t i = 0; i < ld; i++)
		{
			while (int(j) < iv_elem(dimvec, i))
			{
				iv_elem(str, j) = int(i);
				j++;
			}
		}
	}

	res = ivl_new(200);
	if (res == nullptr) goto out_of_memory;
	if (n == 0)
	{
		if (ivl_append(res, str) != 0) goto out_of_memory;
		iv_free(cntvec);
		return res;
	}

	while (1)
	{
		ivector* nstr = iv_new_copy(str);
		if (nstr == nullptr) goto out_of_memory;
		if (ivl_append(res, nstr) != 0)
		{
			iv_free(nstr);
			goto out_of_memory;
		}
		uint32_t j = n - 1;
		iv_elem(cntvec, iv_elem(str, j))++;
		while (j > 0 && iv_elem(str, j - 1) >= iv_elem(str, j))
		{
			j--;
			iv_elem(cntvec, iv_elem(str, j))++;
		}
		if (j == 0) break;

		int a = iv_elem(str, j - 1);
		iv_elem(cntvec, a)++;
		a++;
		while (iv_elem(cntvec, a) == 0) a++;
		iv_elem(str, j - 1) = a;
		iv_elem(cntvec, a)--;

		for (uint32_t i = 0; i < ld; i++)
		{
			for (int k = 0; k < iv_elem(cntvec, i); k++)
			{
				iv_elem(str, j) = int(i);
				j++;
			}
			iv_elem(cntvec, i) = 0;
		}
	}

	iv_free(cntvec);
	iv_free(str);
	return res;

out_of_memory:
	if (cntvec) iv_free(cntvec);
	if (str) iv_free(str);
	if (res) ivl_free_all(res);
	return nullptr;
}

ivlist* all_perms(int n)
{
	assert(n >= 0);
	ivector* dimvec = iv_new(uint32_t(n + 1));
	if (dimvec == nullptr) return nullptr;
	for (uint32_t i = 0; i < iv_length(dimvec); i++) iv_elem(dimvec, i) = int(i);
	ivlist* res = all_strings(dimvec);
	iv_free(dimvec);
	return res;
}

ivector* string2perm(const ivector* str)
{
	uint32_t n = iv_length(str);

	uint32_t N = 0;
	for (uint32_t i = 0; i < n; i++)
		if (int(N) < iv_elem(str, i)) N = uint32_t(iv_elem(str, i));
	N++;

	ivector* dimvec = iv_new_zero(N);
	if (dimvec == nullptr) return nullptr;
	for (uint32_t i = 0; i < n; i++) iv_elem(dimvec, iv_elem(str, i))++;
	for (uint32_t i = 1; i < N; i++) iv_elem(dimvec, i) += iv_elem(dimvec, i - 1);

	ivector* perm = iv_new(n);
	if (perm == nullptr)
	{
		iv_free(dimvec);
		return nullptr;
	}

	for (int i = int(n) - 1; i >= 0; i--)
	{
		int j = iv_elem(str, i);
		iv_elem(dimvec, j)--;
		iv_elem(perm, iv_elem(dimvec, j)) = i + 1;
	}

	iv_free(dimvec);
	return perm;
}

ivector* str2dimvec(const ivector* str)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < iv_length(str); i++)
	{
		if (iv_elem(str, i) < 0) return nullptr;
		if (int(n) <= iv_elem(str, i)) n = uint32_t(iv_elem(str, i)) + 1;
	}
	ivector* res = iv_new_zero(n);
	if (res == nullptr) return nullptr;
	for (uint32_t i = 0; i < iv_length(str); i++) iv_elem(res, iv_elem(str, i))++;
	for (uint32_t i = 1; i < n; i++) iv_elem(res, i) += iv_elem(res, i - 1);
	return res;
}

int str_iscompat(const ivector* str1, const ivector* str2)
{
	if (iv_length(str1) != iv_length(str2)) return 0;
	ivector* dv1 = str2dimvec(str1);
	if (dv1 == nullptr) return 0;
	ivector* dv2 = str2dimvec(str2);
	if (dv2 == nullptr)
	{
		iv_free(dv1);
		return 0;
	}
	int cmp = iv_cmp(dv1, dv2);
	iv_free(dv1);
	iv_free(dv2);
	return (cmp == 0) ? 1 : 0;
}

ivector* perm2string(const ivector* perm, const ivector* dimvec)
{
	int n = iv_length(dimvec) ? iv_elem(dimvec, iv_length(dimvec) - 1) : 0;
	ivector* res = iv_new(uint32_t(n));
	if (res == nullptr) return nullptr;
	uint32_t j = 0;
	for (uint32_t i = 0; i < iv_length(dimvec); i++)
		while (int(j) < iv_elem(dimvec, i))
		{
			int wj = (j < iv_length(perm)) ? iv_elem(perm, j) : int(j) + 1;
			iv_elem(res, wj - 1) = int(i);
			j++;
		}

	return res;
}
