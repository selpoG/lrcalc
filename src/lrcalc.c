/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern int optind;

#include "ivector.h"
#include "ivlincomb.h"
#include "lriter.h"
#include "maple.h"
#include "optshape.h"
#include "part.h"
#include "schur.h"
#include "vectarg.h"

#define PROGNAME "lrcalc"

typedef struct
{
	char* name;
	char* args;
} usage_t;

void cmd_usage(usage_t* cmd)
{
	fprintf(stderr, "Usage: " PROGNAME " %s %s\n", cmd->name, cmd->args);
	exit(1);
}

void cmd_error(usage_t* cmd, char* msg)
{
	fprintf(stderr, PROGNAME " %s: %s\n", cmd->name, msg);
	cmd_usage(cmd);
}

void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	alloc_report();
	exit(1);
}

/***************  MULT  ***************/

usage_t mult_usage = {
	name : "mult",
	args :
	    "[-m] [-r rows] [-c cols] [-q rows,cols] [-f rows,level] "
	    "part1 - part2"
};

void mult_main(int ac, char** av)
{
	ivector *sh1, *sh2;
	ivlincomb* lc;
	int c;
	int opt_maple = 0;
	int opt_rows = -1;
	int opt_cols = -1;
	int opt_fusion = 0;
	int opt_quantum = 0;
	char* p;

	if (ac == 1) cmd_usage(&mult_usage);

	while ((c = getopt(ac, av, "mr:c:q:f:")) != EOF) switch (c)
		{
		case 'm': opt_maple = 1; break;
		case 'r': opt_rows = atoi(optarg); break;
		case 'c': opt_cols = atoi(optarg); break;
		case 'q':
		case 'f':
			if (c == 'q')
				opt_quantum = 1;
			else
				opt_fusion = 1;
			opt_rows = strtol(optarg, &p, 10);
			if (p == NULL || *p != ',') cmd_usage(&mult_usage);
			opt_cols = atoi(p + 1);
			if (opt_rows < 0 || opt_cols < 0) cmd_usage(&mult_usage);
			break;

		default: cmd_usage(&mult_usage);
		}

	sh1 = get_vect_arg(ac, av);
	if (sh1 == NULL || part_valid(sh1) == 0) cmd_error(&mult_usage, "part1 not a valid partition.");
	sh2 = get_vect_arg(ac, av);
	if (sh2 == NULL || part_valid(sh2) == 0) cmd_error(&mult_usage, "part2 not a valid partition.");

	if (opt_fusion || opt_quantum)
		lc = schur_mult_fusion(sh1, sh2, opt_rows, opt_cols);
	else
		lc = schur_mult(sh1, sh2, opt_rows, opt_cols, -1);

	iv_free(sh1);
	iv_free(sh2);
	if (lc == NULL) out_of_memory();

	if (opt_quantum)
		if (opt_maple)
			maple_qprint_lincomb(lc, opt_cols, "s");
		else
			part_qprint_lincomb(lc, opt_cols);
	else if (opt_maple)
		maple_print_lincomb(lc, "s", 1);
	else
		part_print_lincomb(lc);

#ifdef DEBUG_MEMORY
	ivlc_free_all(lc);
#endif
}

/***************  SKEW  ***************/

usage_t skew_usage = {
	name : "skew",
	args : "[-m] [-r rows] outer / inner",
};

void skew_main(int ac, char** av)
{
	ivector *outer, *inner;
	ivlincomb* lc;
	int opt_maple = 0;
	int opt_rows = -1;
	int c;

	if (ac == 1) cmd_usage(&skew_usage);

	while ((c = getopt(ac, av, "mr:")) != EOF) switch (c)
		{
		case 'm': opt_maple = 1; break;
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(&skew_usage);
		}

	outer = get_vect_arg(ac, av);
	if (outer == NULL || part_valid(outer) == 0) cmd_error(&skew_usage, "outer shape not a valid partition.");
	inner = get_vect_arg(ac, av);
	if (inner == NULL || part_valid(inner) == 0) cmd_error(&skew_usage, "inner shape not a valid partition.");

	lc = schur_skew(outer, inner, opt_rows, -1);
	iv_free(inner);
	iv_free(outer);
	if (lc == NULL) out_of_memory();

	if (opt_maple)
		maple_print_lincomb(lc, "s", 1);
	else
		part_print_lincomb(lc);

#ifdef DEBUG_MEMORY
	ivlc_free_all(lc);
#endif
}

/***************  COPROD  ***************/

usage_t coprod_usage = {name : "coprod", args : "[-a] part"};

void coprod_main(int ac, char** av)
{
	ivector *sh, *part;
	ivlincomb* lc;
	int c, i, rows, cols;
	int opt_all = 0;
	ivlc_iter itr;

	if (ac == 1) cmd_usage(&mult_usage);

	while ((c = getopt(ac, av, "a")) != EOF) switch (c)
		{
		case 'a': opt_all = 1; break;
		default: cmd_usage(&mult_usage);
		}

	sh = get_vect_arg(ac, av);
	if (sh == NULL || part_valid(sh) == 0) cmd_error(&mult_usage, "part not a valid partition.");

	rows = part_length(sh);
	cols = part_entry(sh, 0);

	lc = schur_coprod(sh, rows, cols, -1, opt_all);
	iv_free(sh);
	if (lc == NULL) out_of_memory();

	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  (", ivlc_value(&itr));
		part = ivlc_key(&itr);
		for (i = 0; i < rows && iv_elem(part, i) > cols; i++)
		{
			if (i) putchar(',');
			printf("%d", iv_elem(part, i) - cols);
		}
		printf(")  (");
		for (i = rows; i < iv_length(part) && iv_elem(part, i) != 0; i++)
		{
			if (i > rows) putchar(',');
			printf("%d", iv_elem(part, i));
		}
		putchar(')');
		putchar('\n');
	}

#ifdef DEBUG_MEMORY
	ivlc_free_all(lc);
#endif
}

/***************  COEF  ***************/

usage_t coef_usage = {name : "coef", args : "outer - inner1 - inner2"};

void coef_main(int ac, char** av)
{
	ivector *outer, *sh1, *sh2;
	long long coef;
	int c;

	if (ac == 1) cmd_usage(&coef_usage);

	while ((c = getopt(ac, av, "")) != EOF) switch (c)
		{
		default: cmd_usage(&coef_usage);
		}

	outer = get_vect_arg(ac, av);
	if (outer == NULL || part_valid(outer) == 0) cmd_error(&coef_usage, "outer not a valid partition.");
	sh1 = get_vect_arg(ac, av);
	if (sh1 == NULL || part_valid(sh1) == 0) cmd_error(&coef_usage, "inner1 not a valid partition.");
	sh2 = get_vect_arg(ac, av);
	if (sh2 == NULL || part_valid(sh2) == 0) cmd_error(&coef_usage, "inner2 not a valid partition.");

	coef = schur_lrcoef(outer, sh1, sh2);

#ifdef DEBUG_MEMORY
	iv_free(outer);
	iv_free(sh1);
	iv_free(sh2);
#endif

	if (coef >= 0)
		printf("%lld\n", coef);
	else
		out_of_memory();
}

/***************  TAB  ***************/

usage_t tab_usage = {name : "tab", args : "[-r rows] outer / inner"};

void tab_main(int ac, char** av)
{
	ivector *outer, *inner;
	int opt_rows = -1;
	int c;
	lrtab_iter* lrit;

	if (ac == 1) cmd_usage(&tab_usage);

	while ((c = getopt(ac, av, "r:")) != EOF) switch (c)
		{
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(&tab_usage);
		}

	outer = get_vect_arg(ac, av);
	if (outer == NULL || part_valid(outer) == 0) cmd_error(&tab_usage, "outer shape not a valid partition.");
	inner = get_vect_arg(ac, av);
	if (inner == NULL || part_valid(inner) == 0) cmd_error(&tab_usage, "inner shape not a valid partition.");

	lrit = lrit_new(outer, inner, NULL, opt_rows, -1, -1);
	if (lrit == NULL)
	{
		iv_free(outer);
		iv_free(inner);
		out_of_memory();
	}
	for (; lrit_good(lrit); lrit_next(lrit))
	{
		lrit_print_skewtab(lrit, outer, inner);
		printf("\n");
	}

#ifdef DEBUG_MEMORY
	lrit_free(lrit);
	iv_free(outer);
	iv_free(inner);
#endif
}

/***************  MAIN FUNCTION  ***************/

usage_t* lrcalc_commands[] = {&mult_usage, &skew_usage, &coprod_usage, &coef_usage, &tab_usage, NULL};

void main_usage()
{
	usage_t** cmd;
	fprintf(stderr, "Usage:\n");
	for (cmd = lrcalc_commands; *cmd != NULL; cmd++) fprintf(stderr, PROGNAME " %s %s\n", (*cmd)->name, (*cmd)->args);
	exit(1);
}

int main(int ac, char** av)
{
	char* cmd;

	alloc_getenv();

	if (ac < 2) main_usage();

	cmd = av[1];
	if (cmd[0] == 'l' && cmd[1] == 'r') cmd += 2;

	if (strcmp(cmd, "mult") == 0)
		mult_main(ac - 1, av + 1);
	else if (strcmp(cmd, "skew") == 0)
		skew_main(ac - 1, av + 1);
	else if (strcmp(cmd, "coprod") == 0)
		coprod_main(ac - 1, av + 1);
	else if (strcmp(cmd, "coef") == 0)
		coef_main(ac - 1, av + 1);
	else if (strcmp(cmd, "tab") == 0)
		tab_main(ac - 1, av + 1);
	else
		main_usage();

	alloc_report();
	return 0;
}
