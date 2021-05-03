/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char* optarg;

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/maple.hpp"
#include "lrcalc/perm.hpp"
#include "lrcalc/schublib.hpp"
#include "lrcalc/vectarg.hpp"

#define PROGNAME "schubmult"

[[noreturn]] static void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " [-m] [-s] [-r rank] perm1 - perm2\n");
	exit(1);
}

[[noreturn]] static void error(const char* msg)
{
	fprintf(stderr, PROGNAME ": %s\n", msg);
	print_usage();
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

int main(int ac, char** av)
{
	if (ac == 1) print_usage();

	bool opt_maple = false;
	bool opt_string = false;
	int rank = 0;
	int c;
	while ((c = getopt(ac, av, "msr:")) != EOF) switch (c)
		{
		case 'm': opt_maple = true; break;
		case 's': opt_string = true; break;
		case 'r':
			rank = atoi(optarg);
			if (rank < 0) print_usage();
			break;
		default: print_usage();
		}

	iv_ptr w1{get_vect_arg(ac, av)};
	if (!w1) error("perm1 is missing.");
	iv_ptr w2{get_vect_arg(ac, av)};
	if (!w2) error("perm2 is missing.");

	if (rank > 0 && opt_string) error("-s cannot be used with -r.");

	ivlincomb* lc;
	if (opt_string)
	{
		if (rank > 0) error("options -r and -s cannot be used together.");
		if (!str_iscompat(w1.get(), w2.get())) error("incompatible strings.");
		lc = mult_schubert_str(w1.get(), w2.get());
	}
	else
	{
		if (perm_valid(w1.get()) == 0) error("perm1 not a valid permutation.");
		if (perm_valid(w2.get()) == 0) error("perm2 not a valid permutation.");
		lc = mult_schubert(w1.get(), w2.get(), rank);
	}

	if (lc == nullptr) out_of_memory();

	if (opt_maple)
		maple_print_lincomb(lc, "X", 0);
	else
		ivlc_print(lc, 0);

	return 0;
}
