/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivlist.hpp"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include "lrcalc/ivector.hpp"

/* Initialize list structure. */
static int ivl_init(ivlist* lst, size_t sz)
{
	lst->array = new (std::nothrow) ivector*[sz];
	if (lst->array == nullptr) return -1;
	lst->allocated = sz;
	lst->length = 0;
	return 0;
}

ivlist* ivl_new(size_t sz)
{
	auto lst = new ivlist;
	if (lst == nullptr) return nullptr;
	if (ivl_init(lst, sz) != 0)
	{
		delete lst;
		return nullptr;
	}
	return lst;
}

static void ivl_free(ivlist* v)
{
	delete[] v->array;
	delete v;
}

static int ivl_realloc_array(ivlist* lst, size_t sz)
{
	sz *= 2;
	auto array = new (std::nothrow) ivector*[sz];
	if (array == nullptr) return -1;
	memcpy(array, lst->array, lst->length * sizeof(ivector*));
	delete[] lst->array;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

static int ivl_makeroom(ivlist* lst, size_t sz)
{
	if (sz <= lst->allocated) return 0;
	return ivl_realloc_array(lst, sz);
}

bool ivl_append(ivlist* lst, ivector* x)
{
	if (ivl_makeroom(lst, lst->length + 1) != 0) return false;
	lst->array[(lst->length)++] = x;
	return true;
}

ivector* ivl_poplast(ivlist* lst)
{
	assert(lst->length > 0);
	return lst->array[--(lst->length)];
}

void ivl_free_all(ivlist* lst)
{
	for (size_t i = 0; i < ivl_length(lst); i++) iv_free(ivl_elem(lst, i));
	ivl_free(lst);
}
