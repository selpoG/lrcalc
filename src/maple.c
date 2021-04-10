/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include "ivector.h"
#include "ivlincomb.h"
#include "part.h"

#include "maple.h"


static void _maple_print_term(int c, ivector *v, char *letter, int nz)
{
  int i;
  putchar((c < 0) ? '-' : '+');
  c = abs(c);
  printf("%d*%s[", c, letter);

  for (i = 0; i < iv_length(v); i++)
    {
      if (nz && iv_elem(v, i) == 0)
        break;
      if (i > 0)
        putchar(',');
      printf("%d", iv_elem(v, i));
    }
  putchar(']');
}

void maple_print_lincomb(ivlincomb *ht, char *letter, int nz)
{
  ivlc_iter itr;
  putchar('0');
  for (ivlc_first(ht, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      if (ivlc_value(&itr) == 0)
        continue;
      _maple_print_term(ivlc_value(&itr), ivlc_key(&itr), letter, nz);
    }
  putchar('\n');
}

static void _maple_qprint_term(int c, ivector *v, int level, char *letter)
{
  int d, x, i;
  putchar((c < 0) ? '-' : '+');
  c = abs(c);
  d = part_qdegree(v, level);
  printf("%d*q^%d*%s[", c, d, letter);
  for (i = 0; i < iv_length(v); i++)
    {
      x = part_qentry(v, i, d, level);
      if (x == 0)
        break;
      if (i)
        putchar(',');
      printf("%d", x);
    }
  putchar(']');
}

void maple_qprint_lincomb(ivlincomb *lc, int level, char *letter)
{
  ivlc_iter itr;
  putchar('0');
  for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      if (ivlc_value(&itr) == 0)
        continue;
      _maple_qprint_term(ivlc_value(&itr), ivlc_key(&itr), level, letter);
    }
  putchar('\n');
}
