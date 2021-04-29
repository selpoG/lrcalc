#ifndef LRCALC_ILIST_H
#define LRCALC_ILIST_H

#define LIST ilist
#define PREFIX(name) il_##name
#define VALUE_T int
#define SIZE_T size_t

#ifdef LRCALC_ILIST_C
#undef INLINE
#define INLINE CINLINE
#endif

#include "lrcalc/list.tpl.hpp"

#define il_length(lst) ((lst)->length)

#ifdef DEBUG
#define il_elem(lst, i) (*il_pelem(lst, i))
#else
#define il_elem(lst, i) ((lst)->array[i])
#endif

#ifndef LRCALC_ILIST_C
#undef LIST
#undef VALUE_T
#undef PREFIX
#undef SIZE_T
#endif

#endif
