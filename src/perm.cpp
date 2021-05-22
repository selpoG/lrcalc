/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/perm.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

// check w is a permutation of {1, 2, ..., n}
bool perm_valid(ivector* w)
{
	uint32_t n = iv_length(w);
	// change signs of elements of w temporarily,
	// to check each of 1, ..., n appears only once
	for (uint32_t i = 0; i < n; i++)
	{
		int a = abs(iv_elem(w, i)) - 1;
		// w[a] < 0 means a has appeared before
		if (a < 0 || a >= int(n) || iv_elem(w, a) < 0) return false;
		iv_elem(w, a) = -iv_elem(w, a);
	}
	// revert
	for (uint32_t i = 0; i < n; i++) iv_elem(w, i) = -iv_elem(w, i);
	return true;
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

bool dimvec_valid(const ivector* dv)
{
	uint32_t ld = iv_length(dv);
	if (ld == 0) return false;
	if (iv_elem(dv, 0) < 0) return 0;
	for (uint32_t i = 1; i < ld; i++)
		if (iv_elem(dv, i - 1) > iv_elem(dv, i)) return false;
	return true;
}

/* Return true if S_w1 * S_w2 = 0 in H^*(Fl(rank)). */
bool bruhat_zero(const ivector* w1, const ivector* w2, int rank)
{
	int n1 = perm_group(w1);
	int n2 = perm_group(w2);
	if (n1 > rank || n2 > rank) return true;
	if (n1 > n2)
	{
		std::swap(w1, w2);
		std::swap(n1, n2);
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
			if (r1 < r2) return true;
		}
	}
	return false;
}

ivlist* all_strings(const ivector* dimvec)
{
	assert(dimvec_valid(dimvec));

	uint32_t ld = iv_length(dimvec);
	iv_ptr cntvec = iv_create_zero(ld);
	if (!cntvec) return nullptr;
	int n_ = iv_elem(dimvec, ld - 1);
	if (n_ < 0) return nullptr;
	auto n = uint32_t(n_);

	ivl_ptr res;
	iv_ptr str = iv_create(n);
	if (!str) return nullptr;
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

	res.reset(ivl_new(200));
	if (n == 0)
	{
		ivl_append(res.get(), str.release());
		return res.release();
	}

	while (1)
	{
		ivl_append(res.get(), iv_new_copy(str.get()));
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

	return res.release();
}

ivlist* all_perms(int n)
{
	assert(n >= 0);
	iv_ptr dimvec = iv_create(uint32_t(n + 1));
	if (!dimvec) return nullptr;
	for (uint32_t i = 0; i < iv_length(dimvec); i++) iv_elem(dimvec, i) = int(i);
	return all_strings(dimvec.get());
}

ivector* string2perm(const ivector* str)
{
	uint32_t n = iv_length(str);

	uint32_t N = 0;
	for (uint32_t i = 0; i < n; i++)
		if (int(N) < iv_elem(str, i)) N = uint32_t(iv_elem(str, i));
	N++;

	iv_ptr dimvec = iv_create_zero(N);
	if (!dimvec) return nullptr;
	for (uint32_t i = 0; i < n; i++) iv_elem(dimvec, iv_elem(str, i))++;
	for (uint32_t i = 1; i < N; i++) iv_elem(dimvec, i) += iv_elem(dimvec, i - 1);

	iv_ptr perm = iv_create(n);
	if (!perm) return nullptr;

	for (int i = int(n) - 1; i >= 0; i--)
	{
		int j = iv_elem(str, i);
		iv_elem(dimvec, j)--;
		iv_elem(perm, iv_elem(dimvec, j)) = i + 1;
	}

	return perm.release();
}

ivector* str2dimvec(const ivector* str)
{
	uint32_t n = 0;
	for (uint32_t i = 0; i < iv_length(str); i++)
	{
		if (iv_elem(str, i) < 0) return nullptr;
		if (int(n) <= iv_elem(str, i)) n = uint32_t(iv_elem(str, i)) + 1;
	}
	iv_ptr res = iv_create_zero(n);
	if (!res) return nullptr;
	for (uint32_t i = 0; i < iv_length(str); i++) iv_elem(res, iv_elem(str, i))++;
	for (uint32_t i = 1; i < n; i++) iv_elem(res, i) += iv_elem(res, i - 1);
	return res.release();
}

bool str_iscompat(const ivector* str1, const ivector* str2)
{
	if (iv_length(str1) != iv_length(str2)) return false;
	iv_ptr dv1{str2dimvec(str1)};
	if (!dv1) return false;
	iv_ptr dv2{str2dimvec(str2)};
	if (!dv2) return false;
	return iv_cmp(dv1.get(), dv2.get()) == 0;
}

ivector* perm2string(const ivector* perm, const ivector* dimvec)
{
	int n = iv_length(dimvec) ? iv_elem(dimvec, iv_length(dimvec) - 1) : 0;
	iv_ptr res = iv_create(uint32_t(n));
	if (!res) return nullptr;
	uint32_t j = 0;
	for (uint32_t i = 0; i < iv_length(dimvec); i++)
		while (int(j) < iv_elem(dimvec, i))
		{
			int wj = (j < iv_length(perm)) ? iv_elem(perm, j) : int(j) + 1;
			iv_elem(res, wj - 1) = int(i);
			j++;
		}

	return res.release();
}
