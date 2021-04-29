#ifndef LRCALC_LRCOEF_H
#define LRCALC_LRCOEF_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lrcalc/ivector.hpp"

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(const ivector* outer, const ivector* inner, const ivector* content);

#ifdef __cplusplus
}
#endif
#endif
