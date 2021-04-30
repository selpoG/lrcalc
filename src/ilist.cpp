/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ilist.hpp"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int il_init(ilist* lst, size_t sz)
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

void il_free(ilist* v)
{
	free(v->array);
	free(v);
}

static int il__realloc_array(ilist* lst, size_t sz)
{
	sz *= 2;
	auto array = static_cast<int*>(realloc(lst->array, sz * sizeof(int)));
	if (array == nullptr) return -1;
	lst->array = array;
	lst->allocated = sz;
	return 0;
}

static int il_makeroom(ilist* lst, size_t sz)
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
