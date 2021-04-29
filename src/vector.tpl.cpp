/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdarg.h>
#include <stdio.h>

#include "lrcalc/alloc.hpp"

VECTOR* PREFIX(new_init)(SIZE_T length, ...)
{
	va_list ap;

	VECTOR* v = PREFIX(new)(length);
	va_start(ap, length);
	for (SIZE_T i = 0; i < length; i++) v->array[i] = va_arg(ap, VA_VALUE_T);
	va_end(ap);

	return v;
}

VALUE_T PREFIX(sum)(VECTOR* v)
{
	VALUE_T res = 0;
	for (SIZE_T i = 0; i < v->length; i++) res += v->array[i];
	return res;
}

int PREFIX(lesseq)(VECTOR* v1, VECTOR* v2)
{
	SIZE_T i = 0;
	claim(v1->length == v2->length);
	while (i < v1->length && v1->array[i] <= v2->array[i]) i++;
	return (i == v1->length);
}

void PREFIX(mult)(VECTOR* dst, VALUE_T c, VECTOR* src)
{
	claim(dst->length == src->length);
	for (SIZE_T i = 0; i < dst->length; i++) dst->array[i] = c * src->array[i];
}

void PREFIX(div)(VECTOR* dst, VECTOR* src, VALUE_T c)
{
	claim(dst->length == src->length);
	for (SIZE_T i = 0; i < dst->length; i++) dst->array[i] = src->array[i] / c;
}

VALUE_T PREFIX(max)(VECTOR* v)
{
	SIZE_T n = v->length;
	if (n == 0) return MINVALUE;
	VALUE_T m = v->array[n - 1];
	for (auto i = int(n - 2); i >= 0; i--)
		if (m < v->array[i]) m = v->array[i];
	return m;
}

VALUE_T PREFIX(min)(VECTOR* v)
{
	SIZE_T n = v->length;
	if (n == 0) return MAXVALUE;
	VALUE_T m = v->array[n - 1];
	for (auto i = int(n - 2); i >= 0; i--)
		if (m > v->array[i]) m = v->array[i];
	return m;
}

void PREFIX(reverse)(VECTOR* dst, VECTOR* src)
{
	claim(dst->length == src->length);
	SIZE_T n = dst->length;
	SIZE_T n2 = n / 2;
	for (SIZE_T i = 0; i < n2; i++)
	{
		auto tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
}

#ifdef INTEGER_VALUE
VALUE_T PREFIX(gcd)(VECTOR* v)
{
	VALUE_T x = 0;
	for (SIZE_T i = 0; i < v->length; i++)
	{
		VALUE_T y = x;
		x = v->array[i];
		while (y != 0)
		{
			VALUE_T z = x % y;
			x = y;
			y = z;
		}
	}
	return abs(x);
}
#endif

CINLINE void PREFIX(print)(VECTOR* v)
{
	putchar('(');
	for (SIZE_T i = 0; i < v->length; i++)
	{
		if (i) putchar(',');
		printf(VALUE_FMT, v->array[i]);
	}
	putchar(')');
}

void PREFIX(printnl)(VECTOR* v)
{
	PREFIX(print)(v);
	putchar('\n');
}
