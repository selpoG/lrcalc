/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/perm.hpp"

#define PROGNAME "allperms"

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rank\n");
	exit(1);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

int main(int ac, char** av)
{
	if (ac != 2) print_usage();
	int n = atoi(av[1]);
	if (n < 0) print_usage();

	ivlist* lst = all_perms(n);
	if (lst == nullptr) out_of_memory();

	for (uint32_t i = 0; i < ivl_length(lst); i++) iv_printnl(ivl_elem(lst, i));

	ivl_free_all(lst);
}
