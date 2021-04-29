/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"

#include "lrcalc/maple.hpp"

static void _maple_print_term(int c, ivector* v, const char* letter, int nz)
{
	putchar((c < 0) ? '-' : '+');
	c = abs(c);
	printf("%d*%s[", c, letter);

	for (uint32_t i = 0; i < iv_length(v); i++)
	{
		if (nz && iv_elem(v, i) == 0) break;
		if (i > 0) putchar(',');
		printf("%d", iv_elem(v, i));
	}
	putchar(']');
}

void maple_print_lincomb(ivlincomb* ht, const char* letter, int nz)
{
	putchar('0');
	ivlc_iter itr;
	for (ivlc_first(ht, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		_maple_print_term(ivlc_value(&itr), ivlc_key(&itr), letter, nz);
	}
	putchar('\n');
}

static void _maple_qprint_term(int c, ivector* v, int level, const char* letter)
{
	putchar((c < 0) ? '-' : '+');
	c = abs(c);
	int d = part_qdegree(v, level);
	printf("%d*q^%d*%s[", c, d, letter);
	for (uint32_t i = 0; i < iv_length(v); i++)
	{
		int x = part_qentry(v, int(i), d, level);
		if (x == 0) break;
		if (i) putchar(',');
		printf("%d", x);
	}
	putchar(']');
}

void maple_qprint_lincomb(ivlincomb* lc, int level, const char* letter)
{
	ivlc_iter itr;
	putchar('0');
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		_maple_qprint_term(ivlc_value(&itr), ivlc_key(&itr), level, letter);
	}
	putchar('\n');
}
