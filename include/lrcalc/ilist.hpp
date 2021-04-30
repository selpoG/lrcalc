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

	ilist* il_new(size_t sz);

	void il_free(ilist* v);

	int il_append(ilist* lst, int x);

	int il_poplast(ilist* lst);
#ifdef __cplusplus
}
#endif

#endif
