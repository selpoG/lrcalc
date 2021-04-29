/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char* optarg;

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/maple.hpp"
#include "lrcalc/perm.hpp"
#include "lrcalc/schublib.hpp"
#include "lrcalc/vectarg.hpp"

#define PROGNAME "allstrings"

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " dimvec\n");
	exit(1);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

int main(int ac, char** av)
{
	alloc_getenv();

	ivector* dv = get_vect_arg(ac, av);
	if (dv == nullptr) print_usage();
	if (dimvec_valid(dv) == 0) print_usage();

	ivlist* lst = all_strings(dv);
	if (lst == nullptr)
	{
		iv_free(dv);
		out_of_memory();
	}

	for (uint32_t i = 0; i < ivl_length(lst); i++) iv_printnl(ivl_elem(lst, i));

	iv_free(dv);
	ivl_free_all(lst);
	alloc_report();
}
