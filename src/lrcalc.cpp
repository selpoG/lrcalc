/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern int optind;

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/lriter.hpp"
#include "lrcalc/maple.hpp"
#include "lrcalc/optshape.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/schur.hpp"
#include "lrcalc/vectarg.hpp"

#define PROGNAME "lrcalc"

struct usage_t
{
	const char* name;
	const char* args;
};

[[noreturn]] static void cmd_usage(const usage_t* cmd)
{
	fprintf(stderr, "Usage: " PROGNAME " %s %s\n", cmd->name, cmd->args);
	exit(1);
}

[[noreturn]] static void cmd_error(const usage_t* cmd, const char* msg)
{
	fprintf(stderr, PROGNAME " %s: %s\n", cmd->name, msg);
	cmd_usage(cmd);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

/***************  MULT  ***************/

static usage_t mult_usage = {.name = "mult",
                             .args =
                                 "[-m] [-r rows] [-c cols] [-q rows,cols] [-f rows,level] "
                                 "part1 - part2"};

static void mult_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(&mult_usage);

	int opt_maple = 0;
	int opt_rows = -1;
	int opt_cols = -1;
	int opt_fusion = 0;
	int opt_quantum = 0;
	int c;
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
			char* p;
			opt_rows = int(strtol(optarg, &p, 10));
			if (p == nullptr || *p != ',') cmd_usage(&mult_usage);
			opt_cols = atoi(p + 1);
			if (opt_rows < 0 || opt_cols < 0) cmd_usage(&mult_usage);
			break;

		default: cmd_usage(&mult_usage);
		}

	ivector* sh1 = get_vect_arg(ac, av);
	if (sh1 == nullptr || part_valid(sh1) == 0) cmd_error(&mult_usage, "part1 not a valid partition.");
	ivector* sh2 = get_vect_arg(ac, av);
	if (sh2 == nullptr || part_valid(sh2) == 0) cmd_error(&mult_usage, "part2 not a valid partition.");

	ivlincomb* lc;
	if (opt_fusion || opt_quantum)
		lc = schur_mult_fusion(sh1, sh2, opt_rows, opt_cols);
	else
		lc = schur_mult(sh1, sh2, opt_rows, opt_cols, -1);

	iv_free(sh1);
	iv_free(sh2);
	if (lc == nullptr) out_of_memory();

	if (opt_quantum)
		if (opt_maple)
			maple_qprint_lincomb(lc, opt_cols, "s");
		else
			part_qprint_lincomb(lc, opt_cols);
	else if (opt_maple)
		maple_print_lincomb(lc, "s", 1);
	else
		part_print_lincomb(lc);
}

/***************  SKEW  ***************/

static usage_t skew_usage = {
    .name = "skew",
    .args = "[-m] [-r rows] outer / inner",
};

static void skew_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(&skew_usage);

	int opt_maple = 0;
	int opt_rows = -1;
	int c;
	while ((c = getopt(ac, av, "mr:")) != EOF) switch (c)
		{
		case 'm': opt_maple = 1; break;
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(&skew_usage);
		}

	ivector* outer = get_vect_arg(ac, av);
	if (outer == nullptr || part_valid(outer) == 0) cmd_error(&skew_usage, "outer shape not a valid partition.");
	ivector* inner = get_vect_arg(ac, av);
	if (inner == nullptr || part_valid(inner) == 0) cmd_error(&skew_usage, "inner shape not a valid partition.");

	ivlincomb* lc = schur_skew(outer, inner, opt_rows, -1);
	iv_free(inner);
	iv_free(outer);
	if (lc == nullptr) out_of_memory();

	if (opt_maple)
		maple_print_lincomb(lc, "s", 1);
	else
		part_print_lincomb(lc);
}

/***************  COPROD  ***************/

static usage_t coprod_usage = {.name = "coprod", .args = "[-a] part"};

static void coprod_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(&mult_usage);

	int opt_all = 0;
	int c;
	while ((c = getopt(ac, av, "a")) != EOF) switch (c)
		{
		case 'a': opt_all = 1; break;
		default: cmd_usage(&mult_usage);
		}

	ivector* sh = get_vect_arg(ac, av);
	if (sh == nullptr || part_valid(sh) == 0) cmd_error(&mult_usage, "part not a valid partition.");

	uint32_t rows = part_length(sh);
	int cols = part_entry(sh, 0);

	ivlincomb* lc = schur_coprod(sh, int(rows), cols, -1, opt_all);
	iv_free(sh);
	if (lc == nullptr) out_of_memory();

	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  (", ivlc_value(&itr));
		ivector* part = ivlc_key(&itr);
		for (uint32_t i = 0; i < rows && iv_elem(part, i) > cols; i++)
		{
			if (i) putchar(',');
			printf("%d", iv_elem(part, i) - cols);
		}
		printf(")  (");
		for (uint32_t i = rows; i < iv_length(part) && iv_elem(part, i) != 0; i++)
		{
			if (i > rows) putchar(',');
			printf("%d", iv_elem(part, i));
		}
		putchar(')');
		putchar('\n');
	}
}

/***************  COEF  ***************/

static usage_t coef_usage = {.name = "coef", .args = "outer - inner1 - inner2"};

static void coef_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(&coef_usage);

	int c;
	while ((c = getopt(ac, av, "")) != EOF) switch (c)
		{
		default: cmd_usage(&coef_usage);
		}

	ivector* outer = get_vect_arg(ac, av);
	if (outer == nullptr || part_valid(outer) == 0) cmd_error(&coef_usage, "outer not a valid partition.");
	ivector* sh1 = get_vect_arg(ac, av);
	if (sh1 == nullptr || part_valid(sh1) == 0) cmd_error(&coef_usage, "inner1 not a valid partition.");
	ivector* sh2 = get_vect_arg(ac, av);
	if (sh2 == nullptr || part_valid(sh2) == 0) cmd_error(&coef_usage, "inner2 not a valid partition.");

	long long coef = schur_lrcoef(outer, sh1, sh2);

	if (coef >= 0)
		printf("%lld\n", coef);
	else
		out_of_memory();
}

/***************  TAB  ***************/

static usage_t tab_usage = {.name = "tab", .args = "[-r rows] outer / inner"};

static void tab_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(&tab_usage);

	int opt_rows = -1;
	int c;
	while ((c = getopt(ac, av, "r:")) != EOF) switch (c)
		{
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(&tab_usage);
		}

	ivector* outer = get_vect_arg(ac, av);
	if (outer == nullptr || part_valid(outer) == 0) cmd_error(&tab_usage, "outer shape not a valid partition.");
	ivector* inner = get_vect_arg(ac, av);
	if (inner == nullptr || part_valid(inner) == 0) cmd_error(&tab_usage, "inner shape not a valid partition.");

	lrtab_iter* lrit = lrit_new(outer, inner, nullptr, opt_rows, -1, -1);
	if (lrit == nullptr)
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
}

/***************  MAIN FUNCTION  ***************/

static const usage_t* lrcalc_commands[] = {&mult_usage, &skew_usage, &coprod_usage, &coef_usage, &tab_usage, nullptr};

[[noreturn]] static void main_usage()
{
	fprintf(stderr, "Usage:\n");
	for (const usage_t* const* cmd = lrcalc_commands; *cmd != nullptr; cmd++)
		fprintf(stderr, PROGNAME " %s %s\n", (*cmd)->name, (*cmd)->args);
	exit(1);
}

int main(int ac, char** av)
{
	if (ac < 2) main_usage();

	const char* cmd = av[1];
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

	return 0;
}
