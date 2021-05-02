/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/perm.hpp"
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
	exit(1);
}

int main(int ac, char** av)
{
	iv_ptr dv{get_vect_arg(ac, av)};
	if (!dv) print_usage();
	if (dimvec_valid(dv.get()) == 0) print_usage();

	ivl_ptr lst{all_strings(dv.get())};
	if (!lst) out_of_memory();

	for (uint32_t i = 0; i < ivl_length(lst); i++) iv_printnl(ivl_elem(lst, i));
}
