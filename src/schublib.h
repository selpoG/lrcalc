#ifndef _SCHUBLIB_H
#define _SCHUBLIB_H

#include "ivector.h"
#include "ivlincomb.h"

ivlincomb* trans(ivector* w, int vars);
ivlincomb* monk(int i, ivlincomb* slc, int rank);
ivlincomb* mult_poly_schubert(ivlincomb* poly, ivector* perm, int rank);
ivlincomb* mult_schubert(ivector* ww1, ivector* ww2, int rank);
ivlincomb* mult_schubert_str(ivector* str1, ivector* str2);

#endif
