/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ivector.h"
#include "part.h"
#include "ivlincomb.h"

#define _LRITER_C
#include "lriter.h"


lrtab_iter *lrit_new(ivector *outer, ivector *inner, ivector *content,
                         int maxrows, int maxcols, int partsz)
{
  int len, ilen, clen, out0, inn0, out1, inn1, out2;
  int size, maxdepth, c, r, s, array_len;
  lrtab_iter *lrit;
  ivector *cont;

  claim(part_valid(outer));
  claim(inner == NULL || part_valid(inner));
  claim(content == NULL || part_decr(content));

  /* Empty result if inner not contained in outer. */
  if (inner != NULL && part_leq(inner, outer) == 0)
    {
      cont = iv_new(1);
      if (cont == NULL)
        return NULL;
      lrit = ml_malloc(sizeof(lrtab_iter));
      if (lrit == NULL)
        {
          iv_free(cont);
          return NULL;
        }
      lrit->cont = cont;
      lrit->size = -1;
      return lrit;
    }

  len = part_length(outer);
  ilen = (inner == NULL) ? 0 : iv_length(inner);
  if (ilen > len)
    ilen = len;
  clen = (content == NULL) ? 0 : part_length(content);
  out0 = (len == 0) ? 0 : iv_elem(outer, 0);
  claim(maxcols < 0 || ilen == 0 || iv_elem(inner, 0) == 0);

  /* Find number of boxes and maximal tableau entry. */
  size = 0;
  maxdepth = clen;
  for (r = 0; r < len; r++)
    {
      int inn_r = (r < ilen) ? iv_elem(inner, r) : 0;
      int rowsz = iv_elem(outer, r) - inn_r;
      size += rowsz;
      if (rowsz > 0)
        maxdepth++;
    }
  if (maxrows < 0 || maxrows > maxdepth)
    maxrows = maxdepth;

  /* Find size of array. */
  array_len = size + 2;
  if (maxcols >= 0)
    {
      int clim = maxcols - out0;
      int c1 = 0;
      for (r = clen - 1; r >= 0; r--)
        {
          int c0 = iv_elem(content, r);
          if (c1 < c0 && c1 < maxcols && c0 > clim)
            array_len++;
          c1 = c0;
        }
      if (c1 >= maxcols)
        array_len--;
    }

  /* Allocate array. */
  lrit = ml_malloc(sizeof(lrtab_iter) + array_len * sizeof(lrit_box));
  if (lrit == NULL)
    return NULL;
  lrit->array_len = array_len;

  /* Allocate and copy content. */
  if (partsz < maxrows)
    partsz = maxrows;
  cont = lrit->cont = iv_new(partsz);
  if (cont == NULL)
    {
      ml_free(lrit);
      return NULL;
    }
  lrit->size = -1;
  if (maxrows < clen)
    return lrit; /* empty result. */
  for (r = 0; r < clen; r++)
    iv_elem(cont, r) = iv_elem(content, r);
  for (; r < partsz; r++)
    iv_elem(cont, r) = 0;

  /* Check for empty result. */
  if (maxcols >= 0 && clen > 0 && iv_elem(cont, 0) > maxcols)
    return lrit; /* empty result. */
  if (maxcols >= 0 && out0 > maxcols)
    return lrit; /* empty result. */

  /* Initialize box structure. */
  s = 0;
  out1 = 0;
  inn0 = (len == 0) ? out0 : (len <= ilen ? iv_elem(inner, len-1) : 0);
  for (r = len-1; r >= 0; r--)
    {
      out2 = out1;
      inn1 = inn0;
      out1 = iv_elem(outer, r);
      inn0 = (r == 0) ? out0 : (r <= ilen ? iv_elem(inner, r-1) : 0);
      if (inn1 < out1)
        maxdepth--;
      for (c = inn1; c < out1; c++)
        {
          lrit_box *box = lrit->array + s;
          int max;
          box->right = (c+1 < out1) ? (s+1) : (array_len-1);
          box->above = (c >= inn0) ? (s + out1 - inn0) : size;
          max = (c < out2) ? (lrit->array[s-out2+inn1].max - 1) : (maxrows - 1);
          box->max = (max < maxdepth) ? max : maxdepth;
          s++;
        }
    }
  claim(maxdepth == clen);

  /* Set values of extra boxes. */
  lrit->array[array_len-1].value = maxrows - 1;
  lrit->array[size].value = -1;
  if (maxcols >= 0)
    {
      int clim = maxcols - out0;
      int c1 = 0;
      int s = array_len - 2;
      int i = out0;
      for (r = clen - 1; r >= 0; r--)
        {
          int c0 = iv_elem(content, r);
          if (c1 < c0 && c1 < maxcols && c0 > clim)
            {
              lrit->array[s].value = r;
              while (i > maxcols - c0 && i > 0)
                lrit->array[size - out0 + (--i)].above = s;
              s--;
            }
          c1 = c0;
        }
    }

  /* Minimal LR tableau. */
  for (s = size-1; s >= 0; s--)
    {
      lrit_box *box = lrit->array + s;
      int x = lrit->array[box->above].value + 1;
      if (x > box->max)
        return lrit; /* empty result. */
      box->value = x;
      iv_elem(cont, x)++;
    }

  lrit->size = size;
  return lrit;
}


void lrit_print_skewtab(lrtab_iter *lrit, ivector *outer, ivector *inner)
{
  lrit_box *array = lrit->array;
  int size = lrit->size;
  int ilen, len, col_first, r;

  ilen = (inner == NULL) ? 0 : iv_length(inner);
  len = part_length(outer);
  if (len <= ilen)
    while (len > 0 && iv_elem(inner, len-1) == iv_elem(outer, len-1))
      len--;
  if (len == 0)
    return;

  col_first = (ilen < len) ? 0 : iv_elem(inner, len-1);
  r = 0;
  while (r < ilen && iv_elem(inner,r) == iv_elem(outer, r))
    r++;
  for (; r < len; r++)
    {
      int inn_r = (r >= ilen) ? 0 : iv_elem(inner, r);
      int out_r = iv_elem(outer, r);
      int row_sz = out_r - inn_r;
      int c;
      size -= row_sz;
      for (c = col_first; c < inn_r; c++)
        fputs("  ", stdout);
      for (c = 0; c < row_sz; c++)
        printf("%2d", array[size + c].value);
      putchar('\n');
    }
}


void lrit_dump(lrtab_iter *lrit)
{
  int r;
  printf("cont = ");
  iv_printnl(lrit->cont);
  printf("size = %d\n", lrit->size);
  for (r = 0; r < lrit->array_len; r++)
    {
      lrit_box *box = lrit->array + r;
      printf("%d: value=%d, max=%d, above=%d (%d), right=%d (%d)\n",
             r, box->value, box->max,
             box->above, lrit->array[box->above].value,
             box->right, lrit->array[box->right].value);
    }
}


void lrit_dump_skew(lrtab_iter *lrit, ivector *outer, ivector *inner)
{
  lrit_box *array = lrit->array;
  int size, ilen, len, col_first, r, s, array_len;

  printf("cont = ");
  iv_printnl(lrit->cont);
  printf("size = %d\n", lrit->size);

  ilen = (inner == NULL) ? 0 : iv_length(inner);
  len = part_length(outer);
  if (len <= ilen)
    while (len > 0 && iv_elem(inner, len-1) == iv_elem(outer, len-1))
      len--;

  col_first = (len == 0 || ilen < len) ? 0 : iv_elem(inner, len-1);
  r = 0;
  while (r < ilen && iv_elem(inner,r) == iv_elem(outer, r))
    r++;
  size = iv_sum(outer) - iv_sum(inner);
  array_len = lrit->array_len;
  for (s = size; s < array_len; s++)
    printf("  %02d:[%02d]", s, array[s].value);
  putchar('\n');
  for (; r < len; r++)
    {
      int inn_r = (r >= ilen) ? 0 : iv_elem(inner, r);
      int out_r = iv_elem(outer, r);
      int row_sz = out_r - inn_r;
      int c;
      size -= row_sz;
      for (c = col_first; c < inn_r; c++)
        fputs("                  ", stdout);
      for (c = 0; c < row_sz; c++)
        {
          lrit_box *box = array + size + c;
          printf("  %02d:[%02d,%02d,%02d,%02d]", size + c, box->value,
                 box->max, box->right, box->above);
          if (box->right >= array_len)
            array_len = box->right + 1;
        }
      putchar('\n');
    }
}


ivlincomb *lrit_expand(ivector *outer, ivector *inner, ivector *content,
                       int maxrows, int maxcols, int partsz)
{
  lrtab_iter *lrit;
  ivlincomb *lc;
  lrit = lrit_new(outer, inner, content, maxrows, maxcols, partsz);
  if (lrit == NULL)
    return NULL;
  lc = lrit_count(lrit);
  lrit_free(lrit);
  return lc;
}
