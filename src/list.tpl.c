/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdarg.h>

LIST * PREFIX(new_init)(SIZE_T sz, SIZE_T num, ...)
{
  LIST *lst;
  SIZE_T i;
  va_list ap;

  lst = PREFIX(new)(sz);
  va_start(ap, num);
  for (i = 0; i < num; i++)
    PREFIX(append) (lst, va_arg(ap, VALUE_T));
  va_end(ap);

  return lst;
}

int PREFIX(_realloc_array) (LIST *lst, SIZE_T sz)
{
  VALUE_T *array;
  sz *= 2;
  array = ml_realloc(lst->array, sz * sizeof(VALUE_T));
  if (array == NULL)
    return -1;
  lst->array = array;
  lst->allocated = sz;
  return 0;
}

int PREFIX(reverse) (LIST *dst, LIST *src)
{
  SIZE_T i, n, n2;
  n = src->length;
  if (dst != src && PREFIX(makeroom)(dst, n) != 0)
    return -1;
  n2 = n / 2;
  for (i = 0; i < n2; i++)
    {
      VALUE_T tmp = src->array[i];
      dst->array[i] = src->array[n-1 - i];
      dst->array[n-1 - i] = tmp;
    }
  if (n & 1)
    dst->array[n2] = src->array[n2];
  return 0;
}
