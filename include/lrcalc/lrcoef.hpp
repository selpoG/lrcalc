#ifndef LRCALC_LRCOEF_H
#define LRCALC_LRCOEF_H

#include "lrcalc/ivector.hpp"

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(ivector* outer, ivector* inner, ivector* content);

#endif
