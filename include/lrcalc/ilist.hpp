#ifndef LRCALC_ILIST_H
#define LRCALC_ILIST_H
#ifdef __cplusplus
extern "C"
{
#endif

#define LIST ilist
#define PREFIX(name) il_##name
#define VALUE_T int

#include "lrcalc/list.tpl.hpp"

#define il_length(lst) ((lst)->length)

#define il_elem(lst, i) ((lst)->array[i])

#ifndef LRCALC_ILIST_C
#undef LIST
#undef VALUE_T
#undef PREFIX
#endif

#ifdef __cplusplus
}
#endif
#endif
