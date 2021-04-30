/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char* optarg;

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

	int opt_maple = 0;
	int opt_string = 0;
	int rank = 0;
	int c;
	while ((c = getopt(ac, av, "msr:")) != EOF) switch (c)
		{
		case 'm': opt_maple = 1; break;
		case 's': opt_string = 1; break;
		case 'r':
			rank = atoi(optarg);
			if (rank < 0) print_usage();
			break;
		default: print_usage();
		}

	ivector* w1 = get_vect_arg(ac, av);
	if (w1 == nullptr) error("perm1 is missing.");
	ivector* w2 = get_vect_arg(ac, av);
	if (w2 == nullptr) error("perm2 is missing.");

	if (rank > 0 && opt_string) error("-s cannot be used with -r.");

	ivlincomb* lc;
	if (opt_string)
	{
		if (rank > 0) error("options -r and -s cannot be used together.");
		if (str_iscompat(w1, w2) == 0) error("incompatible strings.");
		lc = mult_schubert_str(w1, w2);
	}
	else
	{
		if (perm_valid(w1) == 0) error("perm1 not a valid permutation.");
		if (perm_valid(w2) == 0) error("perm2 not a valid permutation.");
		lc = mult_schubert(w1, w2, rank);
	}

	if (lc == nullptr)
	{
		iv_free(w1);
		iv_free(w2);
		out_of_memory();
	}

	if (opt_maple)
		maple_print_lincomb(lc, "X", 0);
	else
		ivlc_print(lc, 0);

	return 0;
}
