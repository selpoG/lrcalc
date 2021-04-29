/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

#define LRCALC_PERM_C
#include "lrcalc/perm.hpp"

ivlist* all_strings(ivector* dimvec)
{
	claim(dimvec_valid(dimvec));

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
	claim(n >= 0);
	ivector* dimvec = iv_new(uint32_t(n + 1));
	if (dimvec == nullptr) return nullptr;
	for (uint32_t i = 0; i < iv_length(dimvec); i++) iv_elem(dimvec, i) = int(i);
	ivlist* res = all_strings(dimvec);
	iv_free(dimvec);
	return res;
}

ivector* string2perm(ivector* str)
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

ivector* str2dimvec(ivector* str)
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

int str_iscompat(ivector* str1, ivector* str2)
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
