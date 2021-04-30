#ifndef LRCALC_ILIST_H
#define LRCALC_ILIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
	struct ilist
	{
		int* array;
		size_t allocated;
		size_t length;
	};

	/* Initialize list structure. */
	int il_init(ilist* lst, size_t sz);

	ilist* il_new(size_t sz);

	void il_dealloc(ilist* v);

	void il_free(ilist* v);

	void il_reset(ilist* lst);

	int il__realloc_array(ilist* lst, size_t sz);

	int il_makeroom(ilist* lst, size_t sz);

	int il_append(ilist* lst, int x);

	int il_poplast(ilist* lst);

	int il_insert(ilist* lst, size_t i, int x);

	int il_delete(ilist* lst, size_t i);

	int il_fastdelete(ilist* lst, size_t i);

	int il_extend(ilist* dst, const ilist* src);

	int il_copy(ilist* dst, const ilist* src);

	ilist* il_new_copy(const ilist* lst);

	int il_reverse(ilist* dst, const ilist* src);

#define il_length(lst) ((lst)->length)

#define il_elem(lst, i) ((lst)->array[i])
#ifdef __cplusplus
}
#endif

#endif
