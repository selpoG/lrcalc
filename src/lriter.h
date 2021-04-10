#ifndef _LRITER_H
#define _LRITER_H

#include "ivector.h"
#include "ivlincomb.h"

#ifdef _LRITER_C
#undef INLINE
#define INLINE CINLINE
#endif

typedef struct {
  int value;
  int max;
  int above;
  int right;
} lrit_box;

typedef struct {
  ivector *cont;
  int size;
  int array_len;
  lrit_box array[1];
} lrtab_iter;


lrtab_iter *lrit_new(ivector *outer, ivector *inner, ivector *content,
                         int maxrows, int maxcols, int partsz);

INLINE void lrit_free(lrtab_iter *lrit)
{
  iv_free(lrit->cont);
  ml_free(lrit);
}

void lrit_print_skewtab(lrtab_iter *lrit, ivector *outer, ivector *inner);
void lrit_dump(lrtab_iter *lrit);
void lrit_dump_skew(lrtab_iter *lrit, ivector *outer, ivector *inner);


INLINE int lrit_good(lrtab_iter *lrit)
{
  return lrit->size >= 0;
}


INLINE void lrit_next(lrtab_iter *lrit)
{
  ivector *cont = lrit->cont;
  lrit_box *box, *box_bound, *array = lrit->array;
  int size = lrit->size;
  int max, x;
  box_bound = array + size;
  for (box = array; box != box_bound; box++)
    {
      max = array[box->right].value;
      if (max > box->max)
        max = box->max;
      x = box->value;
      iv_elem(cont, x)--;
      x++;
      while (x <= max && iv_elem(cont,x) == iv_elem(cont,x-1))
        x++;
      if (x > max)
        continue;

      /* Refill tableau with minimal values. */
      box->value = x;
      iv_elem(cont, x)++;
      while (box != array)
        {
          box--;
          x = array[box->above].value + 1;
          box->value = x;
          iv_elem(cont, x)++;
        }
      return;
    }
  lrit->size = -1;
}


INLINE ivlincomb *lrit_count(lrtab_iter *lrit)
{
  ivector *cont = lrit->cont;
  ivlincomb *lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
  if (lc == NULL)
    return NULL;
  for (; lrit_good(lrit); lrit_next(lrit))
    if (ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY) != 0)
      {
        ivlc_free_all(lc);
        return NULL;
      }
  return lc;
}


ivlincomb *lrit_expand(ivector *outer, ivector *inner, ivector *content,
                       int maxrows, int maxcols, int partsz);


#endif
