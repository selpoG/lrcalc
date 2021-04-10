/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
extern char *optarg;

#include "alloc.h"
#include "vectarg.h"
#include "perm.h"
#include "schublib.h"
#include "maple.h"
#include "ivlincomb.h"


#define PROGNAME "allstrings"

void print_usage()
{
  fprintf(stderr, "usage: " PROGNAME " dimvec\n");
  exit(1);
}

void out_of_memory()
{
  fprintf(stderr, PROGNAME ": out of memory.\n");
  alloc_report();
  exit(1);
}


int main(int ac, char **av)
{
  ivector *dv;
  ivlist *lst;
  int i;

  alloc_getenv();

  dv = get_vect_arg(ac, av);
  if (dv == NULL)
    print_usage();
  if (dimvec_valid(dv) == 0)
    print_usage();

  lst = all_strings(dv);
  if (lst == NULL)
    {
      iv_free(dv);
      out_of_memory();
    }

  for (i = 0; i < ivl_length(lst); i++)
    iv_printnl(ivl_elem(lst, i));

  iv_free(dv);
  ivl_free_all(lst);
  alloc_report();
}