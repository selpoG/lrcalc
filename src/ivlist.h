#ifndef _IVLIST_H
#define _IVLIST_H

#include "ivector.h"

#define LIST ivlist
#define PREFIX(name) ivl_ ## name
#define VALUE_T ivector *
#define SIZE_T size_t

#ifdef _IVLIST_C
#undef INLINE
#define INLINE CINLINE
#endif

#include "list.tpl.h"

#define ivl_length(lst) ((lst)->length)

#ifdef DEBUG
#define ivl_elem(lst,i) (*ivl_pelem(lst,i))
#else
#define ivl_elem(lst,i) ((lst)->array[i])
#endif

void ivl_free_all(ivlist *lst);

#ifndef _IVLIST_C
#undef LIST
#undef VALUE_T
#undef PREFIX
#undef SIZE_T
#endif

#endif
