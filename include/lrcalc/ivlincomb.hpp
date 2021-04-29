#ifndef LRCALC_IVLINCOMB_H
#define LRCALC_IVLINCOMB_H

#include <stdint.h>

#include "lrcalc/ivector.hpp"

#define HASHTAB ivlincomb
#define PREFIX(name) ivlc_##name
#define KEY_T ivector*
#define HASH_T uint32_t
#define VALUE_T int32_t
#define SIZE_T uint32_t
#define KEY_CMP iv_cmp
#define KEY_COPY iv_new_copy
#define KEY_DEALLOC iv_free
#define KEY_PRINT iv_print
#define VALUE_DEALLOC(v)
#define HASHTAB_LINCOMB

#ifdef LRCALC_IVLINCOMB_C
#undef INLINE
#define INLINE CINLINE
#endif

#include "lrcalc/hashtab.tpl.hpp"

#ifndef LRCALC_IVLINCOMB_C
#undef HASHTAB
#undef PREFIX
#undef KEY_T
#undef VALUE_T
#undef HASH_T
#undef SIZE_T
#undef KEY_CMP
#undef KEY_COPY
#undef KEY_DEALLOC
#undef KEY_PRINT
#undef VALUE_DEALLOC
#endif

#ifndef IVLC_HASHTABLE_SZ
#define IVLC_HASHTABLE_SZ 2003
#endif
#ifndef IVLC_ARRAY_SZ
#define IVLC_ARRAY_SZ 100
#endif

#endif
