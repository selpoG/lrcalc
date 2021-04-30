/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <stdarg.h>

int PREFIX(init)(LIST* lst, size_t sz)
{
	lst->array = static_cast<VALUE_T*>(malloc(sz * sizeof(VALUE_T)));
	if (lst->array == nullptr) return -1;
	lst->allocated = sz;
	lst->length = 0;
	return 0;
}

LIST* PREFIX(new)(size_t sz)
{
	auto lst = static_cast<LIST*>(malloc(sizeof(LIST)));
	if (lst == nullptr) return nullptr;
	if (PREFIX(init)(lst, sz) != 0)
	{
		free(lst);
		return nullptr;
	}
	return lst;
}

LIST* PREFIX(new_init)(size_t sz, size_t num, ...)
{
	va_list ap;

	LIST* lst = PREFIX(new)(sz);
	va_start(ap, num);
	for (size_t i = 0; i < num; i++) PREFIX(append)(lst, va_arg(ap, VALUE_T));
	va_end(ap);

	return lst;
}

void PREFIX(dealloc)(LIST* v) { free(v->array); }

void PREFIX(free)(LIST* v)
{
	free(v->array);
	free(v);
}

void PREFIX(reset)(LIST* lst) { lst->length = 0; }

int PREFIX(_realloc_array)(LIST* lst, size_t sz)
{
	sz *= 2;
	auto array = static_cast<VALUE_T*>(realloc(lst->array, sz * sizeof(VALUE_T)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

int PREFIX(makeroom)(LIST* lst, size_t sz)
{
	if (sz <= lst->allocated)
		return 0;
	else
		return PREFIX(_realloc_array)(lst, sz);
}

int PREFIX(append)(LIST* lst, VALUE_T x)
{
	if (PREFIX(makeroom)(lst, lst->length + 1) != 0) return -1;
	lst->array[(lst->length)++] = x;
	return 0;
}

VALUE_T PREFIX(poplast)(LIST* lst)
{
	assert(lst->length > 0);
	return lst->array[--(lst->length)];
}

int PREFIX(insert)(LIST* lst, size_t i, VALUE_T x)
{
	assert(0 <= i && i <= lst->length);
	if (PREFIX(makeroom)(lst, lst->length + 1) != 0) return -1;
	size_t n = lst->length - i;
	lst->length++;
	memmove(lst->array + i + 1, lst->array + i, n * sizeof(VALUE_T));
	lst->array[i] = x;
	return 0;
}

VALUE_T PREFIX(delete)(LIST* lst, size_t i)
{
	assert(0 <= i && i < lst->length);
	VALUE_T x = lst->array[i];
	lst->length--;
	size_t n = lst->length - i;
	memmove(lst->array + i, lst->array + i + 1, n * sizeof(VALUE_T));
	return x;
}

VALUE_T PREFIX(fastdelete)(LIST* lst, size_t i)
{
	assert(0 <= i && i < lst->length);
	VALUE_T x = lst->array[i];
	lst->array[i] = lst->array[lst->length - 1];
	(lst->length)--;
	return x;
}

int PREFIX(extend)(LIST* dst, const LIST* src)
{
	size_t dlen = dst->length;
	size_t slen = src->length;
	if (PREFIX(makeroom)(dst, dst->length + src->length) != 0) return -1;
	memmove(dst->array + dlen, src->array, slen * sizeof(VALUE_T));
	return 0;
}

int PREFIX(copy)(LIST* dst, const LIST* src)
{
	if (PREFIX(makeroom)(dst, src->length) != 0) return -1;
	dst->length = src->length;
	memcpy(dst->array, src->array, dst->length * sizeof(VALUE_T));
	return 0;
}

LIST* PREFIX(new_copy)(const LIST* lst)
{
	LIST* res = PREFIX(new)(lst->length);
	if (res == nullptr) return nullptr;
	res->length = lst->length;
	memcpy(res->array, lst->array, res->length * sizeof(VALUE_T));
	return res;
}

int PREFIX(reverse)(LIST* dst, const LIST* src)
{
	size_t n = src->length;
	if (dst != src && PREFIX(makeroom)(dst, n) != 0) return -1;
	size_t n2 = n / 2;
	for (size_t i = 0; i < n2; i++)
	{
		VALUE_T tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
	return 0;
}
