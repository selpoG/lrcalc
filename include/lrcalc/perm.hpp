#ifndef LRCALC_PERM_H
#define LRCALC_PERM_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

#ifdef LRCALC_PERM_C
#undef INLINE
#define INLINE CINLINE
#endif

INLINE int perm_valid(ivector* w)
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

INLINE int perm_length(const ivector* w)
{
	uint32_t n = iv_length(w);
	int res = 0;
	for (uint32_t i = 0; i + 1 < n; i++)
		for (uint32_t j = i + 1; j < n; j++)
			if (iv_elem(w, i) > iv_elem(w, j)) res++;
	return res;
}

INLINE int perm_group(const ivector* w)
{
	int i = int(iv_length(w));
	while (i > 0 && iv_elem(w, i - 1) == i) i--;
	return i;
}

INLINE int dimvec_valid(const ivector* dv)
{
	uint32_t ld = iv_length(dv);
	if (ld == 0) return 0;
	if (iv_elem(dv, 0) < 0) return 0;
	for (uint32_t i = 1; i < ld; i++)
		if (iv_elem(dv, i - 1) > iv_elem(dv, i)) return 0;
	return 1;
}

/* Return 1 if w1 <= w2. */
INLINE int bruhat_leq(const ivector* w1, const ivector* w2)
{
	int n = perm_group(w1);
	if (n > perm_group(w2)) return 0;
	for (int q = 1; q < n; q++)
	{
		int r1 = 0;
		int r2 = 0;
		for (int p = 0; p < n - 1; p++)
		{
			if (iv_elem(w1, p) <= q) r1++;
			if (iv_elem(w2, p) <= q) r2++;
			if (r1 < r2) return 0;
		}
	}
	return 1;
}

/* Return 1 if S_w1 * S_w2 = 0 in H^*(Fl(rank)). */
INLINE int bruhat_zero(const ivector* w1, const ivector* w2, int rank)
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

int str_iscompat(const ivector* str1, const ivector* str2);

ivlist* all_strings(const ivector* dimvec);
ivlist* all_perms(int n);

ivector* string2perm(const ivector* str);
ivector* str2dimvec(const ivector* str);
ivector* perm2string(const ivector* perm, const ivector* dimvec);

#endif
