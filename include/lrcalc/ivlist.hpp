#ifndef LRCALC_IVLIST_H
#define LRCALC_IVLIST_H

#include <stddef.h>

#include "lrcalc/ivector.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	struct ivlist
	{
		ivector** array;
		size_t allocated;
		size_t length;
	};

	ivlist* ivl_new(size_t sz);

	void ivl_append(ivlist* lst, ivector* x);

	ivector* ivl_poplast(ivlist* lst);

#define ivl_length(lst) ((lst)->length)

#define ivl_elem(lst, i) ((lst)->array[i])

	void ivl_free_all(ivlist* lst);
#ifdef __cplusplus
}
#endif

#endif
