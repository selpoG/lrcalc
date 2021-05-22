/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/schublib.hpp"

#include <assert.h>
#include <stdint.h>

#include <limits>
#include <new>
#include <utility>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/perm.hpp"

static void _trans(ivector* w, int vars, ivlincomb* res);

ivlincomb* trans(ivector* w, int vars)
{
	ivlc_ptr res = ivlc_create();
	_trans(w, vars, res.get());
	return res.release();
}

static void _trans(ivector* w, int vars, ivlincomb* res)
{
	ivlc_reset(res);

	uint32_t nw = iv_length(w);
	int n = perm_group(w);
	w->length = uint32_t(n);

	int r = n - 1;
	while (r > 0 && iv_elem(w, r - 1) < iv_elem(w, r)) r--;
	if (r <= 0)
	{
		ivector* xx = iv_new_zero(vars ? uint32_t(vars) : 1);
		w->length = nw;
		ivlc_insert(res, xx, iv_hash(xx), 1);
		return;
	}
	if (vars < r) vars = r;

	int s = r + 1;
	while (s < n && iv_elem(w, r - 1) > iv_elem(w, s)) s++;

	int wr = iv_elem(w, r - 1);
	int ws = iv_elem(w, s - 1);

	ivector* v = w;
	iv_elem(v, s - 1) = wr;
	iv_elem(v, r - 1) = ws;

	ivlincomb* tmp = trans(v, vars);
	for (auto& kv : ivlc_iterator(tmp))
	{
		ivector* xx = kv.key;
		iv_elem(xx, r - 1)++;
		uint32_t hash = iv_hash(xx);
		ivlc_insert(res, xx, hash, kv.value);
	}

	int last = 0;
	int vr = iv_elem(v, r - 1);
	for (int i = r - 1; i >= 1; i--)
	{
		int vi = iv_elem(v, i - 1);
		if (last < vi && vi < vr)
		{
			last = vi;
			iv_elem(v, i - 1) = vr;
			iv_elem(v, r - 1) = vi;
			_trans(v, vars, tmp);
			ivlc_add_multiple(res, 1, tmp, LC_FREE_ZERO);
			iv_elem(v, i - 1) = vi;
		}
	}

	w->length = nw;
	iv_elem(w, s - 1) = ws;
	iv_elem(w, r - 1) = wr;
	ivlc_free(tmp);
}

static void _monk_add(uint32_t i, const ivlc_ptr& slc, int rank, ivlc_ptr& res)
{
	for (const auto& kv : ivlc_iterator(slc))
	{
		const ivector* w = kv.key;
		int c = kv.value;
		uint32_t n = iv_length(w);
		int wi = (i <= n) ? iv_elem(w, i - 1) : int(i);

		if (i <= n + 1)
		{
			int last = 0;
			uint32_t ulen = (i > n) ? i : n;
			for (uint32_t j = i - 1; j >= 1; j--)
				if (last < iv_elem(w, j - 1) && iv_elem(w, j - 1) < wi)
				{
					last = iv_elem(w, j - 1);
					ivector* u = iv_new(ulen);
					for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
					for (uint32_t t = n; t < ulen; t++) iv_elem(u, t) = int(t + 1);
					iv_elem(u, j - 1) = wi;
					iv_elem(u, i - 1) = last;
					ivlc_add_element(res.get(), -c, u, iv_hash(u), LC_FREE_ZERO);
				}
		}
		else
		{
			ivector* u = iv_new(i);
			for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
			for (uint32_t t = n; t < i - 2; t++) iv_elem(u, t) = int(t + 1);
			iv_elem(u, i - 2) = int(i);
			iv_elem(u, i - 1) = int(i) - 1;
			ivlc_add_element(res.get(), -c, u, iv_hash(u), LC_FREE_ZERO);
		}

		if (i >= n + 1)
		{
			ivector* u = iv_new(i + 1);
			for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
			for (uint32_t t = n; t < i; t++) iv_elem(u, t) = int(t + 1);
			iv_elem(u, i - 1) = int(i) + 1;
			iv_elem(u, i) = int(i);
			ivlc_add_element(res.get(), c, u, iv_hash(u), LC_FREE_ZERO);
		}
		else
		{
			int last = std::numeric_limits<int>::max();
			for (uint32_t j = i + 1; j <= n; j++)
				if (wi < iv_elem(w, j - 1) && iv_elem(w, j - 1) < last)
				{
					last = iv_elem(w, j - 1);
					ivector* u = iv_new(n);
					for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
					iv_elem(u, i - 1) = last;
					iv_elem(u, j - 1) = wi;
					ivlc_add_element(res.get(), c, u, iv_hash(u), LC_FREE_ZERO);
				}
			if (last > int(n) && int(n) < rank)
			{
				ivector* u = iv_new(n + 1);
				for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
				iv_elem(u, i - 1) = int(n) + 1;
				iv_elem(u, n) = wi;
				ivlc_add_element(res.get(), c, u, iv_hash(u), LC_FREE_ZERO);
			}
		}
	}
}

static void _mult_ps(void** poly, uint32_t n, uint32_t maxvar, const ivector* perm, int rank, ivlc_ptr& res);

ivlincomb* mult_poly_schubert(ivlincomb* poly, ivector* perm, int rank)
{
	ivlc_ptr poly_ptr{poly};
	uint32_t n = ivlc_card(poly_ptr.get());
	if (n == 0) return poly;

	if (rank == 0) rank = std::numeric_limits<int>::max();

	auto p = new (std::nothrow) void*[2 * n];
	if (p == nullptr) return nullptr;
	uint32_t i = 0;
	uint32_t maxvar = 0;
	for (auto& kv : ivlc_iterator(poly_ptr))
	{
		ivector* xx = kv.key;
		uint32_t j = iv_length(xx);
		while (j > 0 && iv_elem(xx, j - 1) == 0) j--;
		xx->length = j;
		if (maxvar < j) maxvar = j;
		p[i++] = kv.key;
		p[i++] = reinterpret_cast<void*>(long(kv.value));
	}
	assert(i == 2 * n);
	ivlc_reset(poly_ptr.get());

	uint32_t svlen = iv_length(perm);
	perm->length = uint32_t(perm_group(perm));
	_mult_ps(p, n, maxvar, perm, rank, poly_ptr);
	perm->length = svlen;

	for (i = 0; i < n; i++) iv_free(static_cast<ivector*>(p[2 * i]));
	delete[] p;

	return poly_ptr.release();
}

static void _mult_ps(void** poly, uint32_t n, uint32_t maxvar, const ivector* perm, int rank, ivlc_ptr& res)
{
	if (maxvar == 0)
	{
		ivector* w = iv_new_copy(perm); /* FIXME: OPTIMIZE! */
		int c = int(reinterpret_cast<long>(poly[1]));
		ivlc_add_element(res.get(), c, w, iv_hash(w), LC_FREE_ZERO);
		return;
	}

	uint32_t mv0 = 0;
	uint32_t mv1 = 0;
	uint32_t j = 0;
	for (uint32_t i = 0; i < n; i++)
	{
		ivector* xx = static_cast<ivector*>(poly[2 * i]);
		uint32_t lnxx = iv_length(xx);
		if (lnxx < maxvar)
		{
			if (mv0 < lnxx) mv0 = lnxx;
		}
		else
		{
			iv_elem(xx, maxvar - 1)--;
			while (lnxx > 0 && iv_elem(xx, lnxx - 1) == 0) lnxx--;
			xx->length = lnxx;
			if (mv1 < lnxx) mv1 = lnxx;
			poly[2 * i] = poly[2 * j];
			poly[2 * j] = xx;
			std::swap(poly[2 * i + 1], poly[2 * j + 1]);
			j++;
		}
	}

	ivlc_ptr res1 = ivlc_create();
	_mult_ps(poly, j, mv1, perm, rank, res1);
	_monk_add(maxvar, res1, rank, res);

	if (j < n) _mult_ps(poly + 2 * j, n - j, mv0, perm, rank, res);
}

ivlincomb* mult_schubert(ivector* w1, ivector* w2, int rank)
{
	int w1len = perm_length(w1);
	int w2len = perm_length(w2);
	if (w1len > w2len)
	{
		std::swap(w1, w2);
		std::swap(w1len, w2len);
	}

	uint32_t svlen1 = iv_length(w1);
	uint32_t svlen2 = iv_length(w2);
	w1->length = uint32_t(perm_group(w1));
	w2->length = uint32_t(perm_group(w2));
	ivlincomb* lc = nullptr;

	if (rank == 0)
		rank = std::numeric_limits<int>::max();
	else if (2 * (w1len + w2len) > rank * (rank - 1) || bruhat_zero(w1, w2, rank))
	{
		lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
		goto free_return;
	}

	{
		ivlincomb* poly = trans(w1, 0);
		lc = mult_poly_schubert(poly, w2, rank);
	}

free_return:
	w1->length = svlen1;
	w2->length = svlen2;
	return lc;
}

ivlincomb* mult_schubert_str(const ivector* str1, const ivector* str2)
{
	assert(str_iscompat(str1, str2));

	iv_ptr dv{str2dimvec(str1)};
	if (!dv) return nullptr;
	iv_ptr w1{string2perm(str1)};
	iv_ptr w2{string2perm(str2)};

	ivlc_ptr lc{mult_schubert(w1.get(), w2.get(), int(iv_length(w1)))};
	if (!lc) return nullptr;

	w1.reset();
	w2.reset();

	ivlc_ptr res = ivlc_create();
	for (const auto& kv : ivlc_iterator(lc))
	{
		auto str = perm2string(kv.key, dv.get());
		ivlc_insert(res.get(), str, iv_hash(str), kv.value);
	}

	return res.release();
}
