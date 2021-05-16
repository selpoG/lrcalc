/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivector.hpp"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <limits>
#include <new>
#include <numeric>

ivector* iv_new(uint32_t length)
{
	auto arr = new (std::nothrow) int32_t[length];
	if (arr == nullptr) return nullptr;
	auto v = new (std::nothrow) ivector;
	if (v == nullptr)
	{
		delete[] arr;
		return nullptr;
	}
	v->length = length;
	v->array = arr;
	return v;
}

ivector* iv_new_zero(uint32_t length)
{
	auto arr = new (std::nothrow) int32_t[length]();
	if (arr == nullptr) return nullptr;
	auto v = new (std::nothrow) ivector;
	if (v == nullptr)
	{
		delete[] arr;
		return nullptr;
	}
	v->length = length;
	v->array = arr;
	return v;
}

void iv_free(ivector* v)
{
	delete[] v->array;
	delete v;
}

ivector* iv_new_copy(const ivector* v)
{
	ivector* vc = iv_new(v->length);
	if (vc == nullptr) return nullptr;
	memcpy(vc->array, v->array, v->length * sizeof(int32_t));
	return vc;
}

void iv_set_zero(ivector* v) { memset(v->array, 0, v->length * sizeof(int32_t)); }

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

int32_t iv_sum(const ivector* v) { return std::accumulate(v->array, v->array + v->length, 0); }

void iv_print(const ivector* v)
{
	putchar('(');
	for (uint32_t i = 0; i < v->length; i++)
	{
		if (i) putchar(',');
		printf("%d", v->array[i]);
	}
	putchar(')');
}

void iv_printnl(const ivector* v)
{
	iv_print(v);
	putchar('\n');
}
