/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char* optarg;

#include "lrcalc/alloc.hpp"
#include "lrcalc/part.hpp"

#define PROGNAME "allparts"

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rows cols\n");
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

	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows <= 0 || cols <= 0) print_usage();

	ivector* p = iv_new(uint32_t(rows));
	if (p == nullptr) out_of_memory();

	part_iter itr;
	pitr_first(&itr, p, rows, cols, nullptr, nullptr, 0, 0);
	for (; pitr_good(&itr); pitr_next(&itr)) iv_printnl(p);

	iv_free(p);
	alloc_report();
}
