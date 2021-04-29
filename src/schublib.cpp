/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/perm.hpp"

#include "lrcalc/schublib.hpp"

#include <memory>
#include <utility>

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
struct ivlc_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free_all(p); }
};
using safe_iv_ptr = std::unique_ptr<ivector, iv_deleter>;
using safe_ivlc_ptr = std::unique_ptr<ivlincomb, ivlc_deleter>;

static int _trans(ivector* w, int vars, ivlincomb* res);

ivlincomb* trans(ivector* w, int vars)
{
	ivlincomb* res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == nullptr) return nullptr;
	if (_trans(w, vars, res) != 0)
	{
		ivlc_free_all(res);
		return nullptr;
	}
	return res;
}

static int _trans(ivector* w, int vars, ivlincomb* res)
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
		if (xx == nullptr) return -1;
		if (ivlc_insert(res, xx, iv_hash(xx), 1) == nullptr)
		{
			iv_free(xx);
			return -1;
		}
		return 0;
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
	if (tmp == nullptr)
	{
		w->length = nw;
		return -1;
	}
	ivlc_iter itr;
	for (ivlc_first(tmp, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivector* xx = ivlc_key(&itr);
		iv_elem(xx, r - 1)++;
		uint32_t hash = iv_hash(xx);
		if (ivlc_insert(res, xx, hash, ivlc_value(&itr)) == nullptr)
		{
			ivlc_free_all(tmp);
			w->length = nw;
			return -1;
		}
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
			int ok = _trans(v, vars, tmp);
			if (ok == 0) ok = ivlc_add_multiple(res, 1, tmp, LC_FREE_ZERO);
			if (ok != 0)
			{
				ivlc_free_all(tmp);
				w->length = nw;
				return -1;
			}
			iv_elem(v, i - 1) = vi;
		}
	}

	w->length = nw;
	iv_elem(w, s - 1) = ws;
	iv_elem(w, r - 1) = wr;
	ivlc_free(tmp);

	return 0;
}

static int _monk_add(uint32_t i, const ivlincomb* slc, int rank, ivlincomb* res);

ivlincomb* monk(uint32_t i, const ivlincomb* slc, int rank)
{
	ivlincomb* res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res == nullptr) return nullptr;
	if (rank == 0) rank = unsigned(-1) >> 1;
	if (_monk_add(i, slc, rank, res) != 0)
	{
		ivlc_free_all(res);
		return nullptr;
	}
	return res;
}

static int _monk_add(uint32_t i, const ivlincomb* slc, int rank, ivlincomb* res)
{
	ivlc_iter itr;
	for (ivlc_first(slc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		const ivector* w = ivlc_key(&itr);
		int c = ivlc_value(&itr);
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
					if (u == nullptr) return -1;
					for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
					for (uint32_t t = n; t < ulen; t++) iv_elem(u, t) = int(t + 1);
					iv_elem(u, j - 1) = wi;
					iv_elem(u, i - 1) = last;
					if (ivlc_add_element(res, -c, u, iv_hash(u), LC_FREE_ZERO) != 0) return -1;
				}
		}
		else
		{
			ivector* u = iv_new(i);
			if (u == nullptr) return -1;
			for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
			for (uint32_t t = n; t < i - 2; t++) iv_elem(u, t) = int(t + 1);
			iv_elem(u, i - 2) = int(i);
			iv_elem(u, i - 1) = int(i) - 1;
			if (ivlc_add_element(res, -c, u, iv_hash(u), LC_FREE_ZERO) != 0) return -1;
		}

		if (i >= n + 1)
		{
			ivector* u = iv_new(i + 1);
			if (u == nullptr) return -1;
			for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
			for (uint32_t t = n; t < i; t++) iv_elem(u, t) = int(t + 1);
			iv_elem(u, i - 1) = int(i) + 1;
			iv_elem(u, i) = int(i);
			if (ivlc_add_element(res, c, u, iv_hash(u), LC_FREE_ZERO) != 0) return -1;
		}
		else
		{
			int last = unsigned(-1) >> 1;
			for (uint32_t j = i + 1; j <= n; j++)
				if (wi < iv_elem(w, j - 1) && iv_elem(w, j - 1) < last)
				{
					last = iv_elem(w, j - 1);
					ivector* u = iv_new(n);
					if (u == nullptr) return -1;
					for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
					iv_elem(u, i - 1) = last;
					iv_elem(u, j - 1) = wi;
					if (ivlc_add_element(res, c, u, iv_hash(u), LC_FREE_ZERO) != 0) return -1;
				}
			if (last > int(n) && int(n) < rank)
			{
				ivector* u = iv_new(n + 1);
				if (u == nullptr) return -1;
				for (uint32_t t = 0; t < n; t++) iv_elem(u, t) = iv_elem(w, t);
				iv_elem(u, i - 1) = int(n) + 1;
				iv_elem(u, n) = wi;
				if (ivlc_add_element(res, c, u, iv_hash(u), LC_FREE_ZERO) != 0) return -1;
			}
		}
	}
	return 0;
}

static int _mult_ps(void** poly, uint32_t n, uint32_t maxvar, const ivector* perm, int rank, ivlincomb* res);

ivlincomb* mult_poly_schubert(ivlincomb* poly, ivector* perm, int rank)
{
	uint32_t n = ivlc_card(poly);
	if (n == 0) return poly;

	if (rank == 0) rank = unsigned(-1) >> 1;

	auto p = static_cast<void**>(ml_malloc(2 * n * sizeof(void*)));
	if (p == nullptr)
	{
		ivlc_free_all(poly);
		return nullptr;
	}
	uint32_t i = 0;
	uint32_t maxvar = 0;
	ivlc_iter itr;
	for (ivlc_first(poly, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		ivector* xx = ivlc_key(&itr);
		uint32_t j = iv_length(xx);
		while (j > 0 && iv_elem(xx, j - 1) == 0) j--;
		xx->length = j;
		if (maxvar < j) maxvar = j;
		p[i++] = ivlc_key(&itr);
		p[i++] = reinterpret_cast<void*>(long(ivlc_value(&itr)));
	}
	claim(i == 2 * n);
	ivlc_reset(poly);

	uint32_t svlen = iv_length(perm);
	perm->length = uint32_t(perm_group(perm));
	int ok = _mult_ps(p, n, maxvar, perm, rank, poly);
	perm->length = svlen;

	for (i = 0; i < n; i++) iv_free(static_cast<ivector*>(p[2 * i]));
	ml_free(p);

	if (ok != 0)
	{
		ivlc_free_all(poly);
		return nullptr;
	}

	return poly;
}

static int _mult_ps(void** poly, uint32_t n, uint32_t maxvar, const ivector* perm, int rank, ivlincomb* res)
{
	if (maxvar == 0)
	{
		ivector* w = iv_new_copy(perm); /* FIXME: OPTIMIZE! */
		if (w == nullptr) return -1;
		int c = int(reinterpret_cast<long>(poly[1]));
		return ivlc_add_element(res, c, w, iv_hash(w), LC_FREE_ZERO);
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

	ivlincomb* res1 = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (res1 == nullptr) return -1;
	int ok = _mult_ps(poly, j, mv1, perm, rank, res1);
	if (ok == 0) ok = _monk_add(maxvar, res1, rank, res);
	ivlc_free_all(res1);

	if (ok == 0 && j < n) ok = _mult_ps(poly + 2 * j, n - j, mv0, perm, rank, res);
	return ok;
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

	if (rank == 0) { rank = unsigned(-1) >> 1; }
	else if (2 * (w1len + w2len) > rank * (rank - 1) || bruhat_zero(w1, w2, rank))
	{
		lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
		goto free_return;
	}

	{
		ivlincomb* poly = trans(w1, 0);
		if (poly == nullptr) goto free_return;
		lc = mult_poly_schubert(poly, w2, rank);
	}

free_return:
	w1->length = svlen1;
	w2->length = svlen2;
	return lc;
}

ivlincomb* mult_schubert_str(const ivector* str1, const ivector* str2)
{
	claim(str_iscompat(str1, str2));

	safe_iv_ptr dv{str2dimvec(str1)};
	if (!dv) return nullptr;
	safe_iv_ptr w1{string2perm(str1)};
	if (!w1) return nullptr;
	safe_iv_ptr w2{string2perm(str2)};
	if (!w2) return nullptr;

	safe_ivlc_ptr lc{mult_schubert(w1.get(), w2.get(), int(iv_length(w1)))};
	if (!lc) return nullptr;

	w1.reset();
	w2.reset();

	safe_ivlc_ptr res{ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)};
	if (!res) return nullptr;
	ivlc_iter itr;
	for (ivlc_first(lc.get(), &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		safe_iv_ptr str{perm2string(ivlc_key(&itr), dv.get())};
		if (!str) return nullptr;
		if (ivlc_insert(res.get(), str.get(), iv_hash(str.get()), ivlc_value(&itr)) == nullptr)
			// ivlc_insert failed and str must be released
			return nullptr;
		else
			// str was inserted by ivlc_insert successfully and str must not be released
			str.release();
	}

	return res.release();
}
