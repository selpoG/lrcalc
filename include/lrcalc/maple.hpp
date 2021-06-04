#ifndef LRCALC_MAPLE_H
#define LRCALC_MAPLE_H

#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	void maple_print_lincomb(const ivlincomb& ht, const char* letter, bool nz);

	void maple_qprint_lincomb(const ivlincomb& lc, int level, const char* letter);
#ifdef __cplusplus
}
#endif

#endif
