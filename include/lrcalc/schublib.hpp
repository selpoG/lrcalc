#ifndef LRCALC_SCHUBLIB_H
#define LRCALC_SCHUBLIB_H

#include <stdint.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	ivlincomb* trans(ivector* w, int vars);
	ivlincomb* monk(uint32_t i, const ivlincomb* slc, int rank);
	ivlincomb* mult_poly_schubert(ivlincomb* poly, ivector* perm, int rank);
	ivlincomb* mult_schubert(ivector* ww1, ivector* ww2, int rank);
	ivlincomb* mult_schubert_str(const ivector* str1, const ivector* str2);
#ifdef __cplusplus
}
#endif

#endif
