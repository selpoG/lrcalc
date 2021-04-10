/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "alloc.h"


#ifdef DEBUG_MEMORY

void *alloc_heap_base = NULL;  /* Reference point in heap. */
size_t alloc_memory_used = 0;  /* Total allocated memory. */
size_t alloc_call_count = 0;   /* # calls to (m/c/re)alloc. */
size_t alloc_fail_number = 0;  /* If nonzero, fail on alloc call #. */
size_t alloc_fail_count = 0;   /* Number of provoked failures. */
size_t alloc_fail_segfault = 0;/* Provoke segfault on alloc failure #. */
int alloc_dirty_segfault = 1;  /* Provoke segfault on dirty memory. */
int alloc_memory_print = 0;    /* Print log of alloc/free calls. */
size_t alloc_trap_index = -1;  /* Call alloc_trap() if seen. */
size_t alloc_trap_number = -1; /* Call alloc_trap() on alloc call #. */

void alloc_reset()
{
  alloc_heap_base = malloc(4);
  free(alloc_heap_base);
  alloc_memory_used = 0;
  alloc_call_count = 0;
  alloc_fail_number = 0;
  alloc_fail_segfault = 0;
  alloc_dirty_segfault = 1;
  alloc_memory_print = 0;
  alloc_trap_index = -1;
  alloc_trap_number = -1;
}

void alloc_getenv()
{
  char *arg;
  alloc_reset();
  if ((arg = getenv("ML_FAIL_NUMBER")))
    alloc_fail_number = atol(arg);
  if ((arg = getenv("ML_FAIL_SEGFAULT")))
    alloc_fail_segfault = atol(arg);
  if ((arg = getenv("ML_MEMORY_PRINT")))
    alloc_memory_print = atol(arg);
}

void alloc_report()
{
  fprintf(stderr, "Memory balance: %ld\n", alloc_memory_used);
}

void alloc_set_print(int prt)
{
  alloc_memory_print = prt;
}

void alloc_set_trap(size_t index)
{
  alloc_trap_index = index;
}

void alloc_set_fail(size_t N, int segfault)
{
  alloc_fail_number = N;
  alloc_fail_segfault = segfault;
}

void alloc_set_trap_number(size_t N)
{
  alloc_trap_number = N;
}

void alloc_trap(void *p)
{
  size_t index = p - alloc_heap_base;
  fprintf(stderr, "Alloc trap: %zu (%p)\n", index, p);
}

static inline int alloc_check_fail()
{
  alloc_call_count++;
  if (alloc_call_count == alloc_trap_number)
    alloc_trap(NULL);
  if (alloc_fail_number == 0 || alloc_call_count < alloc_fail_number)
    return 0;
  if (alloc_fail_count++)
    fprintf(stderr, "malloc called after out-of-memory event.\n");
  else
    fprintf(stderr, "malloc provoke failure.\n");
  claim((alloc_fail_segfault == 0) ||
        (alloc_fail_count < alloc_fail_segfault));
  return 1;
}

static inline void alloc_print(char c, void *p, char *name)
{
  size_t index = p - alloc_heap_base;
  if (alloc_memory_print)
    fprintf(stderr, "#%c: %ld (%p) %s\n", c, index, p, name);
  if (index == alloc_trap_index)
    alloc_trap(p);
}


#define ALIGN 16
#define HEAD_SPACE 3
#define TAIL_SPACE 3

#define ADD_TO_SIZE ((HEAD_SPACE + TAIL_SPACE) * ALIGN)
#define ADD_TO_PTR (HEAD_SPACE * ALIGN)


static void scramble_storage(void *p)
{
  size_t size = *((size_t *) p);
  memset(p + ADD_TO_PTR, 0x99, size);
}

static void init_storage(void *p, size_t size)
{
  unsigned char *s = p;
  size_t idx;
  *((size_t *) p) = size;
  alloc_memory_used += size;
  for (idx = sizeof(size_t); idx < ADD_TO_PTR; idx++)
    s[idx] = 0xa5;
  for (idx = size + ADD_TO_PTR; idx < size + ADD_TO_SIZE; idx++)
    s[idx] = 0xa5;
}

static void check_storage(void *p)
{
  size_t size = *((size_t *) p);
  size_t idx;
  unsigned char *s = p;
  for (idx = sizeof(size_t); idx < ADD_TO_PTR; idx++)
    if (s[idx] != 0xa5)
      {
        fprintf(stderr, "WARNING: Pointer %zu (%p) dirty at index %zu.\n",
                p - alloc_heap_base, p, idx);
        claim(alloc_dirty_segfault == 0);
        return;
      }
  for (idx = size + ADD_TO_PTR; idx < size + ADD_TO_SIZE; idx++)
    if (s[idx] != 0xa5)
      {
        fprintf(stderr, "WARNING: Pointer %zu (%p) dirty at index %zu.\n",
                p - alloc_heap_base, p, idx);
        claim(alloc_dirty_segfault == 0);
        return;
      }
}


void *ml_malloc(size_t size)
{
  void *p = alloc_check_fail() ? NULL : malloc(size + ADD_TO_SIZE);
  alloc_print('a', p, "malloc");
  if (p == NULL)
    return NULL;
  init_storage(p, size);
  scramble_storage(p);
  return p + ADD_TO_PTR;
}

void *ml_calloc(size_t num, size_t size)
{
  void *p = alloc_check_fail() ? NULL : calloc(1, size * num + ADD_TO_SIZE);
  alloc_print('a', p, "calloc");
  if (p == NULL)
    return NULL;
  init_storage(p, size * num);
  return p + ADD_TO_PTR;
}

void *ml_realloc(void *p, size_t size)
{
  size_t orig_size;
  p -= ADD_TO_PTR;
  orig_size = *((size_t *) p);
  check_storage(p);
  alloc_print('f', p, "realloc");
  p = alloc_check_fail() ? NULL : realloc(p, size + ADD_TO_SIZE);
  alloc_print('a', p, "realloc");
  if (p == NULL)
    return NULL;
  alloc_memory_used -= orig_size;
  init_storage(p, size);
  /* Could scramble new part here. */
  return p + ADD_TO_PTR;
}

void ml_free(void *p)
{
  p -= ADD_TO_PTR;
  alloc_memory_used -= *((size_t *) p);
  check_storage(p);
  scramble_storage(p);
  alloc_print('f', p, "free");
  free(p);
}


/* Test behavior of f on out-of-memory event. */
void alloc_test_oom(void (f)(void *), void *arg)
{
  size_t alloc_count, n;
  alloc_getenv();
  f(arg);
  if (alloc_memory_used != 0)
    {
      fprintf(stderr,
              "alloc_test_oom: non-zero memory balance on first run.\n");
      return;
    }

  /* Total allocation calls by f(arg). */
  alloc_count = alloc_call_count;
  fprintf(stderr, "alloc_test_oom: %zu alloc calls\n", alloc_count);

  for (n = 1; n <= alloc_count; n++)
    {
      fprintf(stderr, "%zu ", n);

      /* Simulate out-of-memory on n-th call. */
      alloc_reset();
      alloc_set_fail(n, 0);
      f(arg);

      if (alloc_memory_used != 0)
        {
          /* Leak found, segfault on n-th call. */
          alloc_reset();
          alloc_set_fail(n, 1);
          f(arg);
        }
      else
        {
          /* segfault if alloc called after failed call. */
          alloc_reset();
          alloc_set_fail(n, 2);
          f(arg);
        }
    }
  fprintf(stderr, "\nalloc_test_oom successful\n");
}

#endif /* DEBUG_MEMORY */
