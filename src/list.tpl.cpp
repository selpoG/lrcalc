/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdarg.h>

LIST* PREFIX(new_init)(SIZE_T sz, SIZE_T num, ...)
{
	va_list ap;

	LIST* lst = PREFIX(new)(sz);
	va_start(ap, num);
	for (SIZE_T i = 0; i < num; i++) PREFIX(append)(lst, va_arg(ap, VALUE_T));
	va_end(ap);

	return lst;
}

int PREFIX(_realloc_array)(LIST* lst, SIZE_T sz)
{
	sz *= 2;
	auto array = static_cast<VALUE_T*>(ml_realloc(lst->array, sz * sizeof(VALUE_T)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

int PREFIX(reverse)(LIST* dst, const LIST* src)
{
	SIZE_T n = src->length;
	if (dst != src && PREFIX(makeroom)(dst, n) != 0) return -1;
	SIZE_T n2 = n / 2;
	for (SIZE_T i = 0; i < n2; i++)
	{
		VALUE_T tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
	return 0;
}
