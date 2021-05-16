/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/maple.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"

static void _maple_print_term(int c, const ivector* v, const char* letter, bool nz)
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

void maple_print_lincomb(const ivlincomb* ht, const char* letter, bool nz)
{
	putchar('0');
	for (const auto& kv : ivlc_iterator(ht))
	{
		if (kv.value == 0) continue;
		_maple_print_term(kv.value, kv.key, letter, nz);
	}
	putchar('\n');
}

static void _maple_qprint_term(int c, const ivector* v, int level, const char* letter)
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

void maple_qprint_lincomb(const ivlincomb* lc, int level, const char* letter)
{
	putchar('0');
	for (const auto& kv : ivlc_iterator(lc))
	{
		if (kv.value == 0) continue;
		_maple_qprint_term(kv.value, kv.key, level, letter);
	}
	putchar('\n');
}
