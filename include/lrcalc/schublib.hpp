#ifndef LRCALC_SCHUBLIB_H
#define LRCALC_SCHUBLIB_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

ivlincomb* trans(ivector* w, int vars);
ivlincomb* monk(uint32_t i, ivlincomb* slc, int rank);
ivlincomb* mult_poly_schubert(ivlincomb* poly, ivector* perm, int rank);
ivlincomb* mult_schubert(ivector* ww1, ivector* ww2, int rank);
ivlincomb* mult_schubert_str(ivector* str1, ivector* str2);

#endif
