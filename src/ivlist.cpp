/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivlist.hpp"

#include <assert.h>
#include <stdarg.h>

int ivl_init(ivlist* lst, size_t sz)
{
	lst->array = static_cast<ivector**>(malloc(sz * sizeof(ivector*)));
	if (lst->array == nullptr) return -1;
	lst->allocated = sz;
	lst->length = 0;
	return 0;
}

ivlist* ivl_new(size_t sz)
{
	auto lst = static_cast<ivlist*>(malloc(sizeof(ivlist)));
	if (lst == nullptr) return nullptr;
	if (ivl_init(lst, sz) != 0)
	{
		free(lst);
		return nullptr;
	}
	return lst;
}

ivlist* ivl_new_init(size_t sz, size_t num, ...)
{
	va_list ap;

	ivlist* lst = ivl_new(sz);
	va_start(ap, num);
	for (size_t i = 0; i < num; i++) ivl_append(lst, va_arg(ap, ivector*));
	va_end(ap);

	return lst;
}

void ivl_dealloc(ivlist* v) { free(v->array); }

void ivl_free(ivlist* v)
{
	free(v->array);
	free(v);
}

void ivl_reset(ivlist* lst) { lst->length = 0; }

int ivl__realloc_array(ivlist* lst, size_t sz)
{
	sz *= 2;
	auto array = static_cast<ivector**>(realloc(lst->array, sz * sizeof(ivector*)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

int ivl_makeroom(ivlist* lst, size_t sz)
{
	if (sz <= lst->allocated)
		return 0;
	else
		return ivl__realloc_array(lst, sz);
}

int ivl_append(ivlist* lst, ivector* x)
{
	if (ivl_makeroom(lst, lst->length + 1) != 0) return -1;
	lst->array[(lst->length)++] = x;
	return 0;
}

ivector* ivl_poplast(ivlist* lst)
{
	assert(lst->length > 0);
	return lst->array[--(lst->length)];
}

int ivl_insert(ivlist* lst, size_t i, ivector* x)
{
	assert(i <= lst->length);
	if (ivl_makeroom(lst, lst->length + 1) != 0) return -1;
	size_t n = lst->length - i;
	lst->length++;
	memmove(lst->array + i + 1, lst->array + i, n * sizeof(ivector*));
	lst->array[i] = x;
	return 0;
}

ivector* ivl_delete(ivlist* lst, size_t i)
{
	assert(i < lst->length);
	ivector* x = lst->array[i];
	lst->length--;
	size_t n = lst->length - i;
	memmove(lst->array + i, lst->array + i + 1, n * sizeof(ivector*));
	return x;
}

ivector* ivl_fastdelete(ivlist* lst, size_t i)
{
	assert(i < lst->length);
	ivector* x = lst->array[i];
	lst->array[i] = lst->array[lst->length - 1];
	(lst->length)--;
	return x;
}

int ivl_extend(ivlist* dst, const ivlist* src)
{
	size_t dlen = dst->length;
	size_t slen = src->length;
	if (ivl_makeroom(dst, dst->length + src->length) != 0) return -1;
	memmove(dst->array + dlen, src->array, slen * sizeof(ivector*));
	return 0;
}

int ivl_copy(ivlist* dst, const ivlist* src)
{
	if (ivl_makeroom(dst, src->length) != 0) return -1;
	dst->length = src->length;
	memcpy(dst->array, src->array, dst->length * sizeof(ivector*));
	return 0;
}

ivlist* ivl_new_copy(const ivlist* lst)
{
	ivlist* res = ivl_new(lst->length);
	if (res == nullptr) return nullptr;
	res->length = lst->length;
	memcpy(res->array, lst->array, res->length * sizeof(ivector*));
	return res;
}

int ivl_reverse(ivlist* dst, const ivlist* src)
{
	size_t n = src->length;
	if (dst != src && ivl_makeroom(dst, n) != 0) return -1;
	size_t n2 = n / 2;
	for (size_t i = 0; i < n2; i++)
	{
		ivector* tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
	return 0;
}

void ivl_free_all(ivlist* lst)
{
	for (size_t i = 0; i < ivl_length(lst); i++) iv_free(ivl_elem(lst, i));
	ivl_free(lst);
}
