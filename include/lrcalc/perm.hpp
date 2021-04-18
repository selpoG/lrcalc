#ifndef _PERM_H
#define _PERM_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

#ifdef _PERM_C
#undef INLINE
#define INLINE CINLINE
#endif

INLINE int perm_valid(ivector* w)
{
	int n, i, a;
	n = iv_length(w);
	for (i = 0; i < n; i++)
	{
		a = abs(iv_elem(w, i)) - 1;
		if (a < 0 || a >= n || iv_elem(w, a) < 0) return 0;
		iv_elem(w, a) = -iv_elem(w, a);
	}
	for (i = 0; i < n; i++) iv_elem(w, i) = -iv_elem(w, i);
	return 1;
}

INLINE int perm_length(ivector* w)
{
	int i, j, n, res;
	n = iv_length(w);
	res = 0;
	for (i = 0; i < n - 1; i++)
		for (j = i + 1; j < n; j++)
			if (iv_elem(w, i) > iv_elem(w, j)) res++;
	return res;
}

INLINE int perm_group(ivector* w)
{
	int i = iv_length(w);
	while (i > 0 && iv_elem(w, i - 1) == i) i--;
	return i;
}

INLINE int dimvec_valid(ivector* dv)
{
	int i, ld = iv_length(dv);
	if (ld == 0) return 0;
	if (iv_elem(dv, 0) < 0) return 0;
	for (i = 1; i < ld; i++)
		if (iv_elem(dv, i - 1) > iv_elem(dv, i)) return 0;
	return 1;
}

/* Return 1 if w1 <= w2. */
INLINE int bruhat_leq(ivector* w1, ivector* w2)
{
	int n, p, q, r1, r2;
	n = perm_group(w1);
	if (n > perm_group(w2)) return 0;
	for (q = 1; q < n; q++)
	{
		r1 = 0;
		r2 = 0;
		for (p = 0; p < n - 1; p++)
		{
			if (iv_elem(w1, p) <= q) r1++;
			if (iv_elem(w2, p) <= q) r2++;
			if (r1 < r2) return 0;
		}
	}
	return 1;
}

/* Return 1 if S_w1 * S_w2 = 0 in H^*(Fl(rank)). */
INLINE int bruhat_zero(ivector* w1, ivector* w2, int rank)
{
	int n1, n2, p, q, q2, r1, r2;
	n1 = perm_group(w1);
	n2 = perm_group(w2);
	if (n1 > rank || n2 > rank) return 1;
	if (n1 > n2)
	{
		ivector* tmp = w1;
		w1 = w2;
		w2 = tmp;
		n1 = n2;
	}
	for (q = 1; q < n1; q++)
	{
		q2 = rank - q;
		r1 = 0;
		r2 = 0;
		for (p = 0; p < n1 - 1; p++)
		{
			if (iv_elem(w1, p) <= q) r1++;
			if (iv_elem(w2, p) > q2) r2++;
			if (r1 < r2) return 1;
		}
	}
	return 0;
}

int str_iscompat(ivector* str1, ivector* str2);

ivlist* all_strings(ivector* dimvec);
ivlist* all_perms(int n);

ivector* string2perm(ivector* str);
ivector* str2dimvec(ivector* str);
ivector* perm2string(ivector* perm, ivector* dimvec);

#endif
