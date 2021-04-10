/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "ivector.h"
#include "ivlist.h"

#define _PERM_C
#include "perm.h"


ivlist *all_strings(ivector *dimvec)
{
  int n, ld, i, j, k;
  ivector *str, *cntvec;
  ivlist *res;

  claim(dimvec_valid(dimvec));

  ld = iv_length(dimvec);
  cntvec = iv_new_zero(ld);
  if (cntvec == NULL)
    return NULL;
  n = iv_elem(dimvec, ld - 1);

  res = NULL;
  str = iv_new(n);
  if (str == NULL) goto out_of_memory;
  j = 0;
  for (i = 0; i < ld; i++)
    {
      while (j < iv_elem(dimvec, i))
        {
          iv_elem(str, j) = i;
          j++;
        }
    }

  res = ivl_new(200);
  if (res == NULL) goto out_of_memory;
  if (n == 0)
    {
      if (ivl_append(res, str) != 0)
        goto out_of_memory;
      iv_free(cntvec);
      return res;
    }

  while (1)
    {
      ivector *nstr = iv_new_copy(str);
      if (nstr == NULL) goto out_of_memory;
      if (ivl_append(res, nstr) != 0)
        {
          iv_free(nstr);
          goto out_of_memory;
        }
      j = n-1;
      iv_elem(cntvec, iv_elem(str, j))++;
      while (j > 0 && iv_elem(str, j-1) >= iv_elem(str, j))
        {
          j--;
          iv_elem(cntvec, iv_elem(str, j))++;
        }
      if (j == 0)
        break;

      i = iv_elem(str, j-1);
      iv_elem(cntvec, i)++;
      i++;
      while(iv_elem(cntvec, i) == 0)
        i++;
      iv_elem(str, j-1) = i;
      iv_elem(cntvec, i)--;

      for (i = 0; i < ld; i++)
        {
          for (k = 0; k < iv_elem(cntvec, i); k++)
            {
              iv_elem(str, j) = i;
              j++;
            }
          iv_elem(cntvec, i) = 0;
        }
    }

  iv_free(cntvec);
  iv_free(str);
  return res;

 out_of_memory:
  if (cntvec) iv_free(cntvec);
  if (str) iv_free(str);
  if (res) ivl_free_all(res);
  return NULL;
}

ivlist *all_perms(int n)
{
  ivlist *res;
  ivector *dimvec;
  int i;
  claim(n >= 0);
  dimvec = iv_new(n + 1);
  if (dimvec == NULL)
    return NULL;
  for (i = 0; i < iv_length(dimvec); i++)
    iv_elem(dimvec, i) = i;
  res = all_strings(dimvec);
  iv_free(dimvec);
  return res;
}

ivector *string2perm(ivector *str)
{
  int N, n, i, j;
  ivector *dimvec, *perm;

  n = iv_length(str);

  N = 0;
  for (i = 0; i < n; i++)
    if (N < iv_elem(str, i))
      N = iv_elem(str, i);
  N++;

  dimvec = iv_new_zero(N);
  if (dimvec == NULL)
    return NULL;
  for (i = 0; i < n; i++)
    iv_elem(dimvec, iv_elem(str, i))++;
  for (i = 1; i < N; i++)
    iv_elem(dimvec, i) += iv_elem(dimvec, i-1);

  perm = iv_new(n);
  if (perm == NULL)
    {
      iv_free(dimvec);
      return NULL;
    }

  for (i = n-1; i >= 0; i--)
    {
      j = iv_elem(str, i);
      iv_elem(dimvec, j)--;
      iv_elem(perm, iv_elem(dimvec, j)) = i+1;
    }

  iv_free(dimvec);
  return perm;
}

ivector *str2dimvec(ivector *str)
{
  int i, n;
  ivector *res;
  n = 0;
  for (i = 0; i < iv_length(str); i++)
    {
      if (iv_elem(str, i) < 0)
        return NULL;
      if (n <= iv_elem(str, i))
        n = iv_elem(str, i) + 1;
    }
  res = iv_new_zero(n);
  if (res == NULL)
    return NULL;
  for (i = 0; i < iv_length(str); i++)
    iv_elem(res, iv_elem(str, i))++;
  for (i = 1; i < n; i++)
    iv_elem(res, i) += iv_elem(res, i-1);
  return res;
}

int str_iscompat(ivector *str1, ivector *str2)
{
  ivector *dv1, *dv2;
  int cmp;
  if (iv_length(str1) != iv_length(str2))
    return 0;
  dv1 = str2dimvec(str1);
  if (dv1 == NULL)
    return 0;
  dv2 = str2dimvec(str2);
  if (dv2 == NULL)
    {
      iv_free(dv1);
      return 0;
    }
  cmp = iv_cmp(dv1, dv2);
  iv_free(dv1);
  iv_free(dv2);
  return (cmp == 0) ? 1 : 0;
}

ivector *perm2string(ivector *perm, ivector *dimvec)
{
  int n, i, j;
  ivector *res;

  n = iv_length(dimvec) ? iv_elem(dimvec, iv_length(dimvec) - 1) : 0;
  res = iv_new(n);
  if (res == NULL)
    return NULL;
  j = 0;
  for (i = 0; i < iv_length(dimvec); i++)
    while (j < iv_elem(dimvec, i))
      {
        int wj = (j < iv_length(perm)) ? iv_elem(perm, j) : j+1;
        iv_elem(res, wj - 1) = i;
        j++;
      }

  return res;
}
