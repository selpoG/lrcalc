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

#define PROGNAME "schubmult"

void print_usage()
{
	fprintf(stderr, "usage: " PROGNAME " [-m] [-s] [-r rank] perm1 - perm2\n");
	exit(1);
}

void error(char* msg)
{
	fprintf(stderr, PROGNAME ": %s\n", msg);
	print_usage();
}

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

int main(int ac, char** av)
{
	ivlincomb* lc;
	ivector *w1, *w2;
	int opt_maple = 0;
	int opt_string = 0;
	int rank = 0;
	int c;

	alloc_getenv();

	if (ac == 1) print_usage();

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

	w1 = get_vect_arg(ac, av);
	if (w1 == NULL) error("perm1 is missing.");
	w2 = get_vect_arg(ac, av);
	if (w2 == NULL) error("perm2 is missing.");

	if (rank > 0 && opt_string) error("-s cannot be used with -r.");

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

	if (lc == NULL)
	{
		iv_free(w1);
		iv_free(w2);
		out_of_memory();
	}

	if (opt_maple)
		maple_print_lincomb(lc, "X", 0);
	else
		ivlc_print(lc, 0);

#ifdef DEBUG_MEMORY
	iv_free(w1);
	iv_free(w2);
	ivlc_free_all(lc);
#endif

	alloc_report();
	return 0;
}
