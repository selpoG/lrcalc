#ifndef LRCALC_PERM_H
#define LRCALC_PERM_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	bool perm_valid(ivector* w);

	int perm_length(const ivector* w);

	int perm_group(const ivector* w);

	bool dimvec_valid(const ivector* dv);

	/* Return true if S_w1 * S_w2 = 0 in H^*(Fl(rank)). */
	bool bruhat_zero(const ivector* w1, const ivector* w2, int rank);

	bool str_iscompat(const ivector* str1, const ivector* str2);

	ivlist* all_strings(const ivector* dimvec);
	ivlist* all_perms(int n);

	ivector* string2perm(const ivector* str);
	ivector* str2dimvec(const ivector* str);
	ivector* perm2string(const ivector* perm, const ivector* dimvec);
#ifdef __cplusplus
}
#endif

#endif
