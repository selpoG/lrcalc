/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"

#define _PART_C
#include "lrcalc/part.hpp"

ivector* part_conj(ivector* p)
{
	int np, nc, j, jlim;
	ivector* conj;
	claim(part_valid(p));
	np = part_length(p);
	nc = (np == 0) ? 0 : iv_elem(p, 0);
	conj = iv_new(nc);
	if (conj == NULL) return NULL;
	j = 0;
	while (np > 0)
	{
		for (jlim = iv_elem(p, np - 1); j < jlim; j++) iv_elem(conj, j) = np;
		np--;
	}
	return conj;
}

CINLINE void part_print(ivector* p)
{
	int i;
	putchar('(');
	for (i = 0; i < iv_length(p) && iv_elem(p, i) != 0; i++)
	{
		if (i) putchar(',');
		printf("%d", iv_elem(p, i));
	}
	putchar(')');
}

void part_printnl(ivector* p)
{
	part_print(p);
	putchar('\n');
}

void part_print_lincomb(ivlincomb* lc)
{
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  ", ivlc_value(&itr));
		part_printnl(ivlc_key(&itr));
	}
}

CINLINE void part_qprint(ivector* p, int level)
{
	int d, i, x;
	d = part_qdegree(p, level);
	putchar('(');
	for (i = 0; i < iv_length(p); i++)
	{
		x = part_qentry(p, i, d, level);
		if (x == 0) break;
		if (i) putchar(',');
		printf("%d", x);
	}
	putchar(')');
}

void part_qprintnl(ivector* p, int level)
{
	part_qprint(p, level);
	putchar('\n');
}

void part_qprint_lincomb(ivlincomb* lc, int level)
{
	ivlc_iter itr;
	for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
	{
		if (ivlc_value(&itr) == 0) continue;
		printf("%d  ", ivlc_value(&itr));
		part_qprintnl(ivlc_key(&itr), level);
	}
}
