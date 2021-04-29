#ifndef LRCALC_IVLIST_H
#define LRCALC_IVLIST_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lrcalc/ivector.hpp"

#define LIST ivlist
#define PREFIX(name) ivl_##name
#define VALUE_T ivector*

#include "lrcalc/list.tpl.hpp"

#define ivl_length(lst) ((lst)->length)

#ifdef DEBUG
#define ivl_elem(lst, i) (*ivl_pelem(lst, i))
#else
#define ivl_elem(lst, i) ((lst)->array[i])
#endif

	void ivl_free_all(ivlist* lst);

#ifndef LRCALC_IVLIST_C
#undef LIST
#undef VALUE_T
#undef PREFIX
#endif

#ifdef __cplusplus
}
#endif
#endif
