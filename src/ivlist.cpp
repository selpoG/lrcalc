/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/alloc.hpp"

#define _IVLIST_C
#include "lrcalc/ivlist.hpp"

#include "list.tpl.cpp"

void ivl_free_all(ivlist* lst)
{
	int i;
	for (i = 0; i < ivl_length(lst); i++) iv_free(ivl_elem(lst, i));
	ivl_free(lst);
}
