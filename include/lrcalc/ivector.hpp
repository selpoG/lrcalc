#ifndef LRCALC_IVECTOR_H
#define LRCALC_IVECTOR_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define VECTOR ivector
#define PREFIX(name) iv_##name
#define VALUE_T int32_t
#define SIZE_T uint32_t

#define INTEGER_VALUE
#include "lrcalc/vector.tpl.hpp"
#undef INTEGER_VALUE

#define iv_length(v) ((v)->length)

#ifdef DEBUG
#define iv_elem(v, i) (*iv_pelem(v, i))
#else
#define iv_elem(v, i) ((v)->array[i])
#endif

#ifndef LRCALC_IVECTOR_C
#undef VECTOR
#undef VALUE_T
#undef PREFIX
#undef SIZE_T
#endif

#ifdef __cplusplus
}
#endif
#endif
