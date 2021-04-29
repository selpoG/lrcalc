#ifndef LRCALC_IVLINCOMB_H
#define LRCALC_IVLINCOMB_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"

	/* This structure allows fast lookup, insertion, and reset.
	   Remove is less optimized.  Iterating through the table will be
	   slow if lots of elements are first inserted and then removed.
	*/
	typedef struct
	{
		ivector* key;
		int32_t value;
		uint32_t hash;
		uint32_t next;
	} ivlc_keyval_t;

	typedef struct
	{
		uint32_t* table;     /* Hash table. */
		ivlc_keyval_t* elts; /* List of elements indexed by table. */
		uint32_t card;       /* Number of (key,value) pairs. */
		uint32_t free_elts;  /* First free keyval item, 0 if none. */
		uint32_t elts_len;   /* Number of itmes used or on free_elts list. */
		uint32_t elts_sz;    /* Allocated items on elts. */
		uint32_t table_sz;   /* Allocated hash table size. */
	} ivlincomb;

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

	uint32_t ivlc_card(const ivlincomb* ht);

	/* Initialize hash table structure. */
	int ivlc_init(ivlincomb* ht, uint32_t tabsz, uint32_t eltsz);

	ivlincomb* ivlc_new(uint32_t tabsz, uint32_t eltsz);

	void ivlc_dealloc(ivlincomb* ht);

	void ivlc_free(ivlincomb* ht);

	void ivlc_reset(ivlincomb* ht);

	int ivlc__grow_table(ivlincomb* ht, uint32_t sz);
	int ivlc__grow_elts(ivlincomb* ht, uint32_t sz);

	int ivlc_makeroom(ivlincomb* ht, uint32_t sz);

	/* Return pointer to keyval_t, nullptr if key not in table. */
	ivlc_keyval_t* ivlc_lookup(const ivlincomb* ht, const ivector* key, uint32_t hash);

	/* Call only if key is not in table.  Insert key into table and return
	   a pointer to new value variable, nullptr if memory allocation
	   error. */
	ivlc_keyval_t* ivlc_insert(ivlincomb* ht, ivector* key, uint32_t hash, int32_t value);

	/* Remove key from hashtable; return pointer to removed keyval_t, or nullptr. */
	ivlc_keyval_t* ivlc_remove(ivlincomb* ht, const ivector* key, uint32_t hash);

	/* Return 1 if equal; ignore zero values unless opt_zero != 0. */
	int ivlc_equals(const ivlincomb* ht1, const ivlincomb* ht2, int opt_zero);

	void ivlc_print_stat(const ivlincomb* ht);

	typedef struct
	{
		const ivlincomb* ht;
		size_t index;
		size_t i;
	} ivlc_iter;

	int ivlc_good(const ivlc_iter* itr);

	void ivlc_first(const ivlincomb* ht, ivlc_iter* itr);

	void ivlc_next(ivlc_iter* itr);

	ivector* ivlc_key(const ivlc_iter* itr);

	int32_t ivlc_value(const ivlc_iter* itr);

	ivlc_keyval_t* ivlc_keyval(const ivlc_iter* itr);

	void ivlc_dealloc_refs(ivlincomb* ht);

	void ivlc_dealloc_all(ivlincomb* ht);

	void ivlc_free_all(ivlincomb* ht);

#define LC_COPY_KEY 1
#define LC_FREE_KEY 0

#define LC_FREE_ZERO 2
#define LC_KEEP_ZERO 0

	int ivlc_add_element(ivlincomb* ht, int32_t c, ivector* key, uint32_t hash, int opt);

	int ivlc_add_multiple(ivlincomb* dst, int32_t c, const ivlincomb* src, int opt);

	void ivlc_print(const ivlincomb* ht, int opt_zero);

#ifndef IVLC_HASHTABLE_SZ
#define IVLC_HASHTABLE_SZ 2003
#endif
#ifndef IVLC_ARRAY_SZ
#define IVLC_ARRAY_SZ 100
#endif

#ifdef __cplusplus
}
#endif
#endif
