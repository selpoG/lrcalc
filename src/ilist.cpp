/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ilist.hpp"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int il_init(ilist* lst, size_t sz)
{
	lst->array = static_cast<int*>(malloc(sz * sizeof(int)));
	if (lst->array == nullptr) return -1;
	lst->allocated = sz;
	lst->length = 0;
	return 0;
}

ilist* il_new(size_t sz)
{
	auto lst = static_cast<ilist*>(malloc(sizeof(ilist)));
	if (lst == nullptr) return nullptr;
	if (il_init(lst, sz) != 0)
	{
		free(lst);
		return nullptr;
	}
	return lst;
}

void il_dealloc(ilist* v) { free(v->array); }

void il_free(ilist* v)
{
	free(v->array);
	free(v);
}

void il_reset(ilist* lst) { lst->length = 0; }

int il__realloc_array(ilist* lst, size_t sz)
{
	sz *= 2;
	auto array = static_cast<int*>(realloc(lst->array, sz * sizeof(int)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

int il_makeroom(ilist* lst, size_t sz)
{
	if (sz <= lst->allocated)
		return 0;
	else
		return il__realloc_array(lst, sz);
}

int il_append(ilist* lst, int x)
{
	if (il_makeroom(lst, lst->length + 1) != 0) return -1;
	lst->array[(lst->length)++] = x;
	return 0;
}

int il_poplast(ilist* lst)
{
	assert(lst->length > 0);
	return lst->array[--(lst->length)];
}

int il_insert(ilist* lst, size_t i, int x)
{
	assert(i <= lst->length);
	if (il_makeroom(lst, lst->length + 1) != 0) return -1;
	size_t n = lst->length - i;
	lst->length++;
	memmove(lst->array + i + 1, lst->array + i, n * sizeof(int));
	lst->array[i] = x;
	return 0;
}

int il_delete(ilist* lst, size_t i)
{
	assert(i < lst->length);
	int x = lst->array[i];
	lst->length--;
	size_t n = lst->length - i;
	memmove(lst->array + i, lst->array + i + 1, n * sizeof(int));
	return x;
}

int il_fastdelete(ilist* lst, size_t i)
{
	assert(i < lst->length);
	int x = lst->array[i];
	lst->array[i] = lst->array[lst->length - 1];
	(lst->length)--;
	return x;
}

int il_extend(ilist* dst, const ilist* src)
{
	size_t dlen = dst->length;
	size_t slen = src->length;
	if (il_makeroom(dst, dst->length + src->length) != 0) return -1;
	memmove(dst->array + dlen, src->array, slen * sizeof(int));
	return 0;
}

int il_copy(ilist* dst, const ilist* src)
{
	if (il_makeroom(dst, src->length) != 0) return -1;
	dst->length = src->length;
	memcpy(dst->array, src->array, dst->length * sizeof(int));
	return 0;
}

ilist* il_new_copy(const ilist* lst)
{
	ilist* res = il_new(lst->length);
	if (res == nullptr) return nullptr;
	res->length = lst->length;
	memcpy(res->array, lst->array, res->length * sizeof(int));
	return res;
}

int il_reverse(ilist* dst, const ilist* src)
{
	size_t n = src->length;
	if (dst != src && il_makeroom(dst, n) != 0) return -1;
	size_t n2 = n / 2;
	for (size_t i = 0; i < n2; i++)
	{
		int tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
	return 0;
}
