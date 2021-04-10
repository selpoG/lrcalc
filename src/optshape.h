#ifndef _OPTSHAPE_H
#define _OPTSHAPE_H

#include "alloc.h"
#include "ivector.h"

#ifdef _OPTSHAPE_C
#undef INLINE
#define INLINE CINLINE
#endif

#ifdef UNDEF_DEBUG
#define DEBUG_OPTSHAPE
#endif


void sksh_print(ivector *outer, ivector *inner, ivector *cont);


typedef struct {
  ivector *outer;
  ivector *inner;
  ivector *cont;
  int sign;
} skew_shape;


int optim_mult(skew_shape *ss, ivector *sh1, ivector *sh2,
               int maxrows, int maxcols);

int optim_fusion(skew_shape *ss, ivector *sh1, ivector *sh2,
                 int maxrows, int maxcols);

int optim_skew(skew_shape *ss, ivector *outer, ivector *inner,
               ivector *content, int maxrows);

int optim_coef(skew_shape *ss, ivector *out, ivector *sh1, ivector *sh2);


INLINE void sksh_dealloc(skew_shape *ss)
{
  if (ss->outer != NULL)
    iv_free(ss->outer);
  if (ss->inner != NULL)
    iv_free(ss->inner);
  if (ss->cont != NULL)
    iv_free(ss->cont);
}


#endif
