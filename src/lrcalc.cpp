/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern int optind;

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/lriter.hpp"
#include "lrcalc/maple.hpp"
#include "lrcalc/part.hpp"
#include "lrcalc/schur.hpp"
#include "lrcalc/vectarg.hpp"

#define PROGNAME "lrcalc"

struct usage_t
{
	const char* name;
	const char* args;
};

[[noreturn]] static void cmd_usage(const usage_t& cmd)
{
	fprintf(stderr, "Usage: " PROGNAME " %s %s\n", cmd.name, cmd.args);
	exit(1);
}

[[noreturn]] static void cmd_error(const usage_t& cmd, const char* msg)
{
	fprintf(stderr, PROGNAME " %s: %s\n", cmd.name, msg);
	cmd_usage(cmd);
}

[[noreturn]] static void out_of_memory()
{
	fprintf(stderr, PROGNAME ": out of memory.\n");
	exit(1);
}

/***************  MULT  ***************/

static usage_t mult_usage{.name = "mult",
                          .args =
                              "[-m] [-r rows] [-c cols] [-q rows,cols] [-f rows,level] "
                              "part1 - part2"};

static void mult_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(mult_usage);

	bool opt_maple = false;
	int opt_rows = -1;
	int opt_cols = -1;
	bool opt_fusion = false;
	bool opt_quantum = false;
	int c;
	while ((c = getopt(ac, av, "mr:c:q:f:")) != EOF) switch (c)
		{
		case 'm': opt_maple = true; break;
		case 'r': opt_rows = atoi(optarg); break;
		case 'c': opt_cols = atoi(optarg); break;
		case 'q':
		case 'f':
			if (c == 'q')
				opt_quantum = true;
			else
				opt_fusion = true;
			char* p;
			opt_rows = int(strtol(optarg, &p, 10));
			if (p == nullptr || *p != ',') cmd_usage(mult_usage);
			opt_cols = atoi(p + 1);
			if (opt_rows < 0 || opt_cols < 0) cmd_usage(mult_usage);
			break;

		default: cmd_usage(mult_usage);
		}

	ivlc_ptr lc;
	{
		iv_ptr sh1{get_vect_arg(ac, av)};
		if (!sh1 || !part_valid(sh1.get())) cmd_error(mult_usage, "part1 not a valid partition.");
		iv_ptr sh2{get_vect_arg(ac, av)};
		if (!sh2 || !part_valid(sh2.get())) cmd_error(mult_usage, "part2 not a valid partition.");

		if (opt_fusion || opt_quantum)
			lc.reset(schur_mult_fusion(sh1.get(), sh2.get(), opt_rows, opt_cols));
		else
			lc.reset(schur_mult(sh1.get(), sh2.get(), opt_rows, opt_cols, -1));
	}
	if (!lc) out_of_memory();

	if (opt_quantum)
		if (opt_maple)
			maple_qprint_lincomb(lc.get(), opt_cols, "s");
		else
			part_qprint_lincomb(lc.get(), opt_cols);
	else if (opt_maple)
		maple_print_lincomb(lc.get(), "s", true);
	else
		part_print_lincomb(lc.get());
}

/***************  SKEW  ***************/

static usage_t skew_usage{
    .name = "skew",
    .args = "[-m] [-r rows] outer / inner",
};

static void skew_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(skew_usage);

	bool opt_maple = false;
	int opt_rows = -1;
	int c;
	while ((c = getopt(ac, av, "mr:")) != EOF) switch (c)
		{
		case 'm': opt_maple = true; break;
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(skew_usage);
		}

	ivlc_ptr lc;
	{
		iv_ptr outer{get_vect_arg(ac, av)};
		if (!outer || !part_valid(outer.get())) cmd_error(skew_usage, "outer shape not a valid partition.");
		iv_ptr inner{get_vect_arg(ac, av)};
		if (!inner || !part_valid(inner.get())) cmd_error(skew_usage, "inner shape not a valid partition.");

		lc.reset(schur_skew(outer.get(), inner.get(), opt_rows, -1));
	}
	if (!lc) out_of_memory();

	if (opt_maple)
		maple_print_lincomb(lc.get(), "s", true);
	else
		part_print_lincomb(lc.get());
}

/***************  COPROD  ***************/

static usage_t coprod_usage{.name = "coprod", .args = "[-a] part"};

static void coprod_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(mult_usage);

	bool opt_all = false;
	int c;
	while ((c = getopt(ac, av, "a")) != EOF) switch (c)
		{
		case 'a': opt_all = true; break;
		default: cmd_usage(mult_usage);
		}

	ivlc_ptr lc;
	uint32_t rows;
	int cols;
	{
		iv_ptr sh{get_vect_arg(ac, av)};
		if (!sh || !part_valid(sh.get())) cmd_error(mult_usage, "part not a valid partition.");

		rows = part_length(sh.get());
		cols = part_entry(sh.get(), 0);

		lc.reset(schur_coprod(sh.get(), int(rows), cols, -1, opt_all));
	}
	if (!lc) out_of_memory();

	ivlc_print_coprod(lc.get(), rows, cols);
}

/***************  COEF  ***************/

static usage_t coef_usage{.name = "coef", .args = "outer - inner1 - inner2"};

static void coef_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(coef_usage);

	int c;
	while ((c = getopt(ac, av, "")) != EOF) switch (c)
		{
		default: cmd_usage(coef_usage);
		}

	iv_ptr outer{get_vect_arg(ac, av)};
	if (!outer || !part_valid(outer.get())) cmd_error(coef_usage, "outer not a valid partition.");
	iv_ptr sh1{get_vect_arg(ac, av)};
	if (!sh1 || !part_valid(sh1.get())) cmd_error(coef_usage, "inner1 not a valid partition.");
	iv_ptr sh2{get_vect_arg(ac, av)};
	if (!sh2 || !part_valid(sh2.get())) cmd_error(coef_usage, "inner2 not a valid partition.");

	long long coef = schur_lrcoef(outer.get(), sh1.get(), sh2.get());

	if (coef >= 0)
		printf("%lld\n", coef);
	else
		out_of_memory();
}

/***************  TAB  ***************/

static usage_t tab_usage{.name = "tab", .args = "[-r rows] outer / inner"};

static void tab_main(int ac, char* const* av)
{
	if (ac == 1) cmd_usage(tab_usage);

	int opt_rows = -1;
	int c;
	while ((c = getopt(ac, av, "r:")) != EOF) switch (c)
		{
		case 'r': opt_rows = atoi(optarg); break;
		default: cmd_usage(tab_usage);
		}

	iv_ptr outer{get_vect_arg(ac, av)};
	if (!outer || !part_valid(outer.get())) cmd_error(tab_usage, "outer shape not a valid partition.");
	iv_ptr inner{get_vect_arg(ac, av)};
	if (!inner || !part_valid(inner.get())) cmd_error(tab_usage, "inner shape not a valid partition.");

	lrtab_iter* lrit = lrit_new(outer.get(), inner.get(), nullptr, opt_rows, -1, -1);
	for (; lrit_good(lrit); lrit_next(lrit))
	{
		lrit_print_skewtab(lrit, outer.get(), inner.get());
		puts_r("");
	}
	lrit_free(lrit);
}

/***************  MAIN FUNCTION  ***************/

static const usage_t* lrcalc_commands[] = {&mult_usage, &skew_usage, &coprod_usage, &coef_usage, &tab_usage};

[[noreturn]] static void main_usage()
{
	fprintf(stderr, "Usage:\n");
	for (const auto& cmd : lrcalc_commands) fprintf(stderr, PROGNAME " %s %s\n", cmd->name, cmd->args);
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
