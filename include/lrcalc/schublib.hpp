#ifndef LRCALC_SCHUBLIB_H
#define LRCALC_SCHUBLIB_H

#include <stdint.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	ivlincomb* trans(const ivector& w, int vars);
	ivlincomb* mult_schubert(ivector& ww1, ivector& ww2, int rank);
	ivlincomb* mult_schubert_str(const ivector& str1, const ivector& str2);
#ifdef __cplusplus
}
#endif

#endif
