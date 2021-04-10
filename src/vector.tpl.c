/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdio.h>
#include <stdarg.h>

#include "alloc.h"


VECTOR * PREFIX(new_init) (SIZE_T length, ...)
{
  VECTOR *v;
  va_list ap;
  SIZE_T i;

  v = PREFIX(new)(length);
  va_start(ap, length);
  for (i = 0; i < length; i++)
    v->array[i] = va_arg(ap, VA_VALUE_T);
  va_end(ap);

  return v;
}

VALUE_T PREFIX(sum) (VECTOR *v)
{
  SIZE_T i;
  VALUE_T res = 0;
  for (i = 0; i < v->length; i++)
    res += v->array[i];
  return res;
}

int PREFIX(lesseq) (VECTOR *v1, VECTOR *v2)
{
  SIZE_T i = 0;
  claim(v1->length == v2->length);
  while (i < v1->length && v1->array[i] <= v2->array[i])
    i++;
  return (i == v1->length);
}

void PREFIX(mult) (VECTOR *dst, VALUE_T c, VECTOR *src)
{
  SIZE_T i;
  claim(dst->length == src->length);
  for (i = 0; i < dst->length; i++)
    dst->array[i] = c * src->array[i];
}

void PREFIX(div)(VECTOR *dst, VECTOR *src, VALUE_T c)
{
  SIZE_T i;
  claim(dst->length == src->length);
  for (i = 0; i < dst->length; i++)
    dst->array[i] = src->array[i] / c;
}

VALUE_T PREFIX(max) (VECTOR *v)
{
  SIZE_T i, n;
  VALUE_T m;
  n = v->length;
  if (n == 0)
    return MINVALUE;
  m = v->array[n-1];
  for (i = n-2; i >= 0; i--)
    if (m < v->array[i])
      m = v->array[i];
  return m;
}

VALUE_T PREFIX(min) (VECTOR *v)
{
  SIZE_T i, n;
  VALUE_T m;
  n = v->length;
  if (n == 0)
    return MAXVALUE;
  m = v->array[n-1];
  for (i = n-2; i >= 0; i--)
    if (m > v->array[i])
      m = v->array[i];
  return m;
}

void PREFIX(reverse) (VECTOR *dst, VECTOR *src)
{
  SIZE_T i, n, n2;
  claim(dst->length == src->length);
  n = dst->length;
  n2 = n / 2;
  for (i = 0; i < n2; i++)
    {
      VALUE_T tmp = src->array[i];
      dst->array[i] = src->array[n-1 - i];
      dst->array[n-1 - i] = tmp;
    }
  if (n & 1)
    dst->array[n2] = src->array[n2];
}

#ifdef INTEGER_VALUE
VALUE_T PREFIX(gcd) (VECTOR *v)
{
  VALUE_T x, y, z;
  SIZE_T i;
  x = 0;
  for (i = 0; i < v->length; i++)
    {
      y = x;
      x = v->array[i];
      while (y != 0)
        {
          z = x % y;
          x = y;
          y = z;
        }
    }
  return abs(x);
}
#endif

CINLINE void PREFIX(print) (VECTOR *v)
{
  SIZE_T i;
  putchar('(');
  for (i = 0; i < v->length; i++)
    {
      if (i)
        putchar(',');
      printf(VALUE_FMT, v->array[i]);
    }
  putchar(')');
}

void PREFIX(printnl) (VECTOR *v)
{
  PREFIX(print)(v);
  putchar('\n');
}
