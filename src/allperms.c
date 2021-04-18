/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char* optarg;

#include "alloc.h"
#include "ivlincomb.h"
#include "maple.h"
#include "perm.h"
#include "schublib.h"
#include "vectarg.h"

#define PROGNAME "allperms"

void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rank\n");
	exit(1);
}

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

int main(int ac, char** av)
{
	int n, i;
	ivlist* lst;

	alloc_getenv();

	if (ac != 2) print_usage();
	n = atoi(av[1]);
	if (n < 0) print_usage();

	lst = all_perms(n);
	if (lst == NULL) out_of_memory();

	for (i = 0; i < ivl_length(lst); i++) iv_printnl(ivl_elem(lst, i));

	ivl_free_all(lst);
	alloc_report();
}
