#include <stdio.h>

#include "lrcalc/alloc.hpp"

/* This structure allows fast lookup, insertion, and reset.
   Remove is less optimized.  Iterating through the table will be
   slow if lots of elements are first inserted and then removed.
*/

typedef struct
{
	KEY_T key;
	VALUE_T value;
	HASH_T hash;
	SIZE_T next;
} PREFIX(keyval_t);

typedef struct
{
	SIZE_T* table;           /* Hash table. */
	PREFIX(keyval_t) * elts; /* List of elements indexed by table. */
	SIZE_T card;             /* Number of (key,value) pairs. */
	SIZE_T free_elts;        /* First free keyval item, 0 if none. */
	SIZE_T elts_len;         /* Number of itmes used or on free_elts list. */
	SIZE_T elts_sz;          /* Allocated items on elts. */
	SIZE_T table_sz;         /* Allocated hash table size. */
} HASHTAB;

/* Minimal number of table entries for each element. */
#ifndef USE_FACTOR
#define USE_FACTOR 2
#endif

#ifndef INIT_TABLE_SIZE
#define INIT_TABLE_SIZE 2003
#endif

#ifndef INIT_ELT_SIZE
#define INIT_ELT_SIZE 100
#endif

SIZE_T PREFIX(card)(const HASHTAB* ht);

/* Initialize hash table structure. */
int PREFIX(init)(HASHTAB* ht, SIZE_T tabsz, SIZE_T eltsz);

HASHTAB* PREFIX(new)(SIZE_T tabsz, SIZE_T eltsz);

void PREFIX(dealloc)(HASHTAB* ht);

void PREFIX(free)(HASHTAB* ht);

void PREFIX(reset)(HASHTAB* ht);

int PREFIX(_grow_table)(HASHTAB* ht, SIZE_T sz);
int PREFIX(_grow_elts)(HASHTAB* ht, SIZE_T sz);

int PREFIX(makeroom)(HASHTAB* ht, SIZE_T sz);

/* Return pointer to keyval_t, nullptr if key not in table. */
PREFIX(keyval_t) * PREFIX(lookup)(const HASHTAB* ht, const KEY_T key, HASH_T hash);

/* Call only if key is not in table.  Insert key into table and return
   a pointer to new value variable, nullptr if memory allocation
   error. */
PREFIX(keyval_t) * PREFIX(insert)(HASHTAB* ht, KEY_T key, HASH_T hash, VALUE_T value);

/* Remove key from hashtable; return pointer to removed keyval_t, or nullptr. */
PREFIX(keyval_t) * PREFIX(remove)(HASHTAB* ht, const KEY_T key, HASH_T hash);

/* Return 1 if equal; ignore zero values unless opt_zero != 0. */
int PREFIX(equals)(const HASHTAB* ht1, const HASHTAB* ht2, int opt_zero);

void PREFIX(print_stat)(const HASHTAB* ht);

typedef struct
{
	const HASHTAB* ht;
	size_t index;
	size_t i;
} PREFIX(iter);

int PREFIX(good)(const PREFIX(iter) * itr);

void PREFIX(first)(const HASHTAB* ht, PREFIX(iter) * itr);

void PREFIX(next)(PREFIX(iter) * itr);

KEY_T PREFIX(key)(const PREFIX(iter) * itr);

VALUE_T PREFIX(value)(const PREFIX(iter) * itr);

PREFIX(keyval_t) * PREFIX(keyval)(const PREFIX(iter) * itr);

void PREFIX(dealloc_refs)(HASHTAB* ht);

void PREFIX(dealloc_all)(HASHTAB* ht);

void PREFIX(free_all)(HASHTAB* ht);

#ifdef HASHTAB_LINCOMB

#define LC_COPY_KEY 1
#define LC_FREE_KEY 0

#define LC_FREE_ZERO 2
#define LC_KEEP_ZERO 0

int PREFIX(add_element)(HASHTAB* ht, VALUE_T c, KEY_T key, HASH_T hash, int opt);

int PREFIX(add_multiple)(HASHTAB* dst, VALUE_T c, const HASHTAB* src, int opt);

void PREFIX(print)(const HASHTAB* ht, int opt_zero);

#endif
