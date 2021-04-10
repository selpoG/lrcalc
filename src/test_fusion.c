/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "alloc.h"
#include "part.h"
#include "schur.h"
#include "perm.h"
#include "ivlincomb.h"
#include "schublib.h"


#define PROGNAME "test_fusion"

void print_usage()
{
  fprintf(stderr, "usage: " PROGNAME " rows cols\n");
  exit(1);
}

void out_of_memory()
{
  fprintf(stderr, PROGNAME ": out of memory.\n");
  alloc_report();
  exit(1);
}


int test_mult_fusion(ivector *sh1, ivector *sh2, int rows, int level)
{
  ivlincomb *prd_f, *prd_s;

  prd_f = prd_s = NULL;

  prd_f = schur_mult_fusion(sh1, sh2, rows, level);
  if (prd_f == NULL) goto out_of_mem;

  prd_s = schur_mult(sh1, sh2, rows, -1, rows);
  if (prd_s == NULL) goto out_of_mem;

  if (fusion_reduce_lc(prd_s, level) != 0)
    goto out_of_mem;

  assert(ivlc_equals(prd_f, prd_s, 0));

  ivlc_free_all(prd_f);
  ivlc_free_all(prd_s);
  return 0;

 out_of_mem:
  if (prd_f) ivlc_free_all(prd_f);
  if (prd_s) ivlc_free_all(prd_s);
  return -1;
}


int main(int ac, char **av)
{
  int rows, cols, level;
  ivector *sh1, *sh2;
  part_iter itr1, itr2;

  alloc_getenv();

  if (ac != 3)
    print_usage();
  rows = atoi(av[1]);
  cols = atoi(av[2]);
  if (rows < 0 || cols < 0)
    print_usage();

  sh1 = iv_new(rows);
  if (sh1 == NULL)
    out_of_memory();
  sh2 = iv_new(rows);
  if (sh2 == NULL)
    {
      iv_free(sh2);
      out_of_memory();
    }

  pitr_box_first(&itr1, sh1, rows, cols);
  for (; pitr_good(&itr1); pitr_next(&itr1))
    {
      pitr_box_first(&itr2, sh2, rows, cols);
      for (; pitr_good(&itr2); pitr_next(&itr2))
        for (level = 0; level <= cols; level++)
          {
            if (test_mult_fusion(sh1, sh2, rows, level) != 0)
              {
                iv_free(sh1);
                iv_free(sh2);
                out_of_memory();
              }
        }
    }

  puts("success");
  iv_free(sh1);
  iv_free(sh2);
  alloc_report();
  return 0;
}
