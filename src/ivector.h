#ifndef _IVECTOR_H
#define _IVECTOR_H

#include <stdint.h>

#define VECTOR ivector
#define PREFIX(name) iv_ ## name
#define VALUE_T int32_t
#define SIZE_T uint32_t

#ifdef _IVECTOR_C
#undef INLINE
#define INLINE CINLINE
#endif

#define INTEGER_VALUE
#include "vector.tpl.h"
#undef INTEGER_VALUE

#define iv_length(v) ((v)->length)

#ifdef DEBUG
#define iv_elem(v,i) (*iv_pelem(v,i))
#else
#define iv_elem(v,i) ((v)->array[i])
#endif

#ifndef _IVECTOR_C
#undef VECTOR
#undef VALUE_T
#undef PREFIX
#undef SIZE_T
#endif

#endif
