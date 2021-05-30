#ifndef LRCALC_OPTSHAPE_H
#define LRCALC_OPTSHAPE_H

#include "lrcalc/ivector.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	void sksh_print(const ivector* outer, const ivector* inner, const ivector* cont);

	struct skew_shape
	{
		ivector* outer;
		ivector* inner;
		ivector* cont;
		int sign;
	};

	skew_shape optim_mult(const ivector* sh1, const ivector* sh2, int maxrows, int maxcols);

	skew_shape optim_fusion(const ivector* sh1, const ivector* sh2, int maxrows, int maxcols);

	skew_shape optim_skew(const ivector* outer, const ivector* inner, const ivector* content, int maxrows);

	skew_shape optim_coef(const ivector* out, const ivector* sh1, const ivector* sh2);

	void sksh_dealloc(skew_shape* ss);
#ifdef __cplusplus
}
#endif

#endif
