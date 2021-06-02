#ifndef LRCALC_PERM_H
#define LRCALC_PERM_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	bool perm_valid(const ivector* w);

	int perm_group(const ivector* w);

	bool dimvec_valid(const ivector* dv);

	bool str_iscompat(const ivector* str1, const ivector* str2);

	ivlist* all_strings(const ivector* dimvec);
	ivlist* all_perms(int n);
#ifdef __cplusplus
}
#endif

#endif
