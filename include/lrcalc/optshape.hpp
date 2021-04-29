#ifndef LRCALC_OPTSHAPE_H
#define LRCALC_OPTSHAPE_H

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"

#ifdef LRCALC_OPTSHAPE_C
#undef INLINE
#define INLINE CINLINE
#endif

#ifdef UNDEF_DEBUG
#define DEBUG_OPTSHAPE
#endif

void sksh_print(const ivector* outer, const ivector* inner, const ivector* cont);

struct skew_shape
{
	ivector* outer;
	ivector* inner;
	ivector* cont;
	int sign;
};

int optim_mult(skew_shape* ss, const ivector* sh1, const ivector* sh2, int maxrows, int maxcols);

int optim_fusion(skew_shape* ss, const ivector* sh1, const ivector* sh2, int maxrows, int maxcols);

int optim_skew(skew_shape* ss, const ivector* outer, const ivector* inner, const ivector* content, int maxrows);

int optim_coef(skew_shape* ss, const ivector* out, const ivector* sh1, const ivector* sh2);

INLINE void sksh_dealloc(skew_shape* ss)
{
	if (ss->outer != nullptr) iv_free(ss->outer);
	if (ss->inner != nullptr) iv_free(ss->inner);
	if (ss->cont != nullptr) iv_free(ss->cont);
}

#endif
