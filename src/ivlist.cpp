/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivlist.hpp"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lrcalc/ivector.hpp"

/* Initialize list structure. */
static int ivl_init(ivlist* lst, size_t sz)
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

static void ivl_free(ivlist* v)
{
	free(v->array);
	free(v);
}

static int ivl__realloc_array(ivlist* lst, size_t sz)
{
	sz *= 2;
	auto array = static_cast<ivector**>(realloc(lst->array, sz * sizeof(ivector*)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

static int ivl_makeroom(ivlist* lst, size_t sz)
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

void ivl_free_all(ivlist* lst)
{
	for (size_t i = 0; i < ivl_length(lst); i++) iv_free(ivl_elem(lst, i));
	ivl_free(lst);
}
