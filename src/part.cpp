/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"

#define LRCALC_PART_C
#include "lrcalc/part.hpp"

ivector* part_conj(const ivector* p)
{
	claim(part_valid(p));
	uint32_t np = part_length(p);
	int nc = (np == 0) ? 0 : iv_elem(p, 0);
	ivector* conj = iv_new(uint32_t(nc));
	if (conj == nullptr) return nullptr;
	uint32_t j = 0;
	while (np > 0)
	{
		for (int jlim = iv_elem(p, np - 1); int(j) < jlim; j++) iv_elem(conj, j) = int(np);
		np--;
	}
	return conj;
}

CINLINE void part_print(const ivector* p)
{
	putchar('(');
	for (uint32_t i = 0; i < iv_length(p) && iv_elem(p, i) != 0; i++)
	{
		if (i) putchar(',');
		printf("%d", iv_elem(p, i));
	}
	putchar(')');
}

void part_printnl(const ivector* p)
{
	part_print(p);
	putchar('\n');
}

void part_print_lincomb(const ivlincomb* lc)
{
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  ", ivlc_value(&itr));
		part_printnl(ivlc_key(&itr));
	}
}

CINLINE void part_qprint(const ivector* p, int level)
{
	int d = part_qdegree(p, level);
	putchar('(');
	for (uint32_t i = 0; i < iv_length(p); i++)
	{
		int x = part_qentry(p, int(i), d, level);
		if (x == 0) break;
		if (i) putchar(',');
		printf("%d", x);
	}
	putchar(')');
}

void part_qprintnl(const ivector* p, int level)
{
	part_qprint(p, level);
	putchar('\n');
}

void part_qprint_lincomb(const ivlincomb* lc, int level)
{
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  ", ivlc_value(&itr));
		part_qprintnl(ivlc_key(&itr), level);
	}
}
