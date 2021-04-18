#ifndef _LRCOEF_H
#define _LRCOEF_H

#include "ivector.h"

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(ivector* outer, ivector* inner, ivector* content);

#endif
