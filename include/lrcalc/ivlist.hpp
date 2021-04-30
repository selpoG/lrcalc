#ifndef LRCALC_IVLIST_H
#define LRCALC_IVLIST_H

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

	/* Initialize list structure. */
	int ivl_init(ivlist* lst, size_t sz);

	ivlist* ivl_new(size_t sz);

	ivlist* ivl_new_init(size_t sz, size_t count, ...);

	void ivl_dealloc(ivlist* v);

	void ivl_free(ivlist* v);

	void ivl_reset(ivlist* lst);

	int ivl__realloc_array(ivlist* lst, size_t sz);

	int ivl_makeroom(ivlist* lst, size_t sz);

	int ivl_append(ivlist* lst, ivector* x);

	ivector* ivl_poplast(ivlist* lst);

	int ivl_insert(ivlist* lst, size_t i, ivector* x);

	ivector* ivl_delete(ivlist* lst, size_t i);

	ivector* ivl_fastdelete(ivlist* lst, size_t i);

	int ivl_extend(ivlist* dst, const ivlist* src);

	int ivl_copy(ivlist* dst, const ivlist* src);

	ivlist* ivl_new_copy(const ivlist* lst);

	int ivl_reverse(ivlist* dst, const ivlist* src);

#define ivl_length(lst) ((lst)->length)

#define ivl_elem(lst, i) ((lst)->array[i])

	void ivl_free_all(ivlist* lst);
#ifdef __cplusplus
}
#endif

#endif
