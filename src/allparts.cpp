/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/part.hpp"

#define PROGNAME "allparts"

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " rows cols\n");
	exit(1);
}

int main(int ac, char** av)
{
	if (ac != 3) print_usage();
	int rows = atoi(av[1]);
	int cols = atoi(av[2]);
	if (rows <= 0 || cols <= 0) print_usage();

	iv_ptr p = iv_create(uint32_t(rows));

	for ([[maybe_unused]] auto& itr : pitr(*p, rows, cols, nullptr, nullptr, 0, 0)) iv_printnl(*p);
}
