#ifndef LRCALC_SCHUR_H
#define LRCALC_SCHUR_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	ivlincomb* schur_mult(const ivector& sh1, const ivector* sh2, int rows, int cols, int partsz);

	ivlincomb* schur_mult_fusion(const ivector& sh1, const ivector& sh2, int rows, int level);

	ivlincomb* schur_skew(const ivector& outer, const ivector* inner, int rows, int partsz);

	ivlincomb* schur_coprod(const ivector& sh, int rows, int cols, int partsz, bool all);

	long long schur_lrcoef(const ivector& outer, const ivector& inner1, const ivector& inner2);
#ifdef __cplusplus
}
#endif

#endif
