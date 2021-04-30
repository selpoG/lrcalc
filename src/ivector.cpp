/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

#include "lrcalc/ivector.hpp"

#define MINVALUE INT_MIN
#define MAXVALUE INT_MAX
#define VA_VALUE_T int
#define VALUE_FMT "%d"

ivector* iv_new(uint32_t length)
{
	auto arr = static_cast<int32_t*>(malloc(length * sizeof(int32_t)));
	if (arr == nullptr) return nullptr;
	auto v = static_cast<ivector*>(malloc(sizeof(ivector)));
	if (v == nullptr)
	{
		free(arr);
		return nullptr;
	}
	v->length = length;
	v->array = arr;
	return v;
}

ivector* iv_new_zero(uint32_t length)
{
	auto arr = static_cast<int32_t*>(calloc(length, sizeof(int32_t)));
	if (arr == nullptr) return nullptr;
	auto v = static_cast<ivector*>(malloc(sizeof(ivector)));
	if (v == nullptr)
	{
		free(arr);
		return nullptr;
	}
	v->length = length;
	v->array = arr;
	return v;
}

ivector* iv_new_init(uint32_t length, ...)
{
	va_list ap;

	ivector* v = iv_new(length);
	va_start(ap, length);
	for (uint32_t i = 0; i < length; i++) v->array[i] = va_arg(ap, VA_VALUE_T);
	va_end(ap);

	return v;
}

void iv_free(ivector* v)
{
	free(v->array);
	free(v);
}

ivector* iv_new_copy(const ivector* v)
{
	ivector* vc = iv_new(v->length);
	if (vc == nullptr) return nullptr;
	memcpy(vc->array, v->array, v->length * sizeof(int32_t));
	return vc;
}

void iv_set_zero(ivector* v) { memset(v->array, 0, v->length * sizeof(int32_t)); }

void iv_copy(ivector* d, const ivector* s)
{
	assert(d->length == s->length);
	memcpy(d->array, s->array, d->length * sizeof(int32_t));
}

int iv_cmp(const ivector* v1, const ivector* v2)
{
	if (v1->length != v2->length) return int(v1->length) - int(v2->length);
	for (uint32_t i = 0; i < v1->length; i++)
		if (v1->array[i] != v2->array[i]) return v1->array[i] - v2->array[i];
	return 0;
}

uint32_t iv_hash(const ivector* v)
{
	uint32_t h = v->length;
	for (uint32_t i = 0; i < v->length; i++) h = ((h << 5) ^ (h >> 27)) + uint32_t(v->array[i]);
	return h;
}

int32_t iv_sum(const ivector* v)
{
	int32_t res = 0;
	for (uint32_t i = 0; i < v->length; i++) res += v->array[i];
	return res;
}

int iv_lesseq(const ivector* v1, const ivector* v2)
{
	uint32_t i = 0;
	assert(v1->length == v2->length);
	while (i < v1->length && v1->array[i] <= v2->array[i]) i++;
	return (i == v1->length);
}

void iv_mult(ivector* dst, int32_t c, const ivector* src)
{
	assert(dst->length == src->length);
	for (uint32_t i = 0; i < dst->length; i++) dst->array[i] = c * src->array[i];
}

void iv_div(ivector* dst, const ivector* src, int32_t c)
{
	assert(dst->length == src->length);
	for (uint32_t i = 0; i < dst->length; i++) dst->array[i] = src->array[i] / c;
}

int32_t iv_max(const ivector* v)
{
	uint32_t n = v->length;
	if (n == 0) return MINVALUE;
	int32_t m = v->array[n - 1];
	for (auto i = int(n - 2); i >= 0; i--)
		if (m < v->array[i]) m = v->array[i];
	return m;
}

int32_t iv_min(const ivector* v)
{
	uint32_t n = v->length;
	if (n == 0) return MAXVALUE;
	int32_t m = v->array[n - 1];
	for (auto i = int(n - 2); i >= 0; i--)
		if (m > v->array[i]) m = v->array[i];
	return m;
}

void iv_reverse(ivector* dst, const ivector* src)
{
	assert(dst->length == src->length);
	uint32_t n = dst->length;
	uint32_t n2 = n / 2;
	for (uint32_t i = 0; i < n2; i++)
	{
		auto tmp = src->array[i];
		dst->array[i] = src->array[n - 1 - i];
		dst->array[n - 1 - i] = tmp;
	}
	if (n & 1) dst->array[n2] = src->array[n2];
}

#ifdef INTEGER_VALUE
int32_t iv_gcd(const ivector* v)
{
	int32_t x = 0;
	for (uint32_t i = 0; i < v->length; i++)
	{
		int32_t y = x;
		x = v->array[i];
		while (y != 0)
		{
			int32_t z = x % y;
			x = y;
			y = z;
		}
	}
	return abs(x);
}
#endif

void iv_print(const ivector* v)
{
	putchar('(');
	for (uint32_t i = 0; i < v->length; i++)
	{
		if (i) putchar(',');
		printf(VALUE_FMT, v->array[i]);
	}
	putchar(')');
}

void iv_printnl(const ivector* v)
{
	iv_print(v);
	putchar('\n');
}
