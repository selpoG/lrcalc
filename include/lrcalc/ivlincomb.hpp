#ifndef LRCALC_IVLINCOMB_H
#define LRCALC_IVLINCOMB_H

#include <stddef.h>
#include <stdint.h>

#include "lrcalc/ivector.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	/* This structure allows fast lookup, insertion, and reset.
	   Remove is less optimized.  Iterating through the table will be
	   slow if lots of elements are first inserted and then removed.
	*/
	struct ivlc_keyval_t
	{
		ivector* key;
		int32_t value;
		uint32_t hash;
		uint32_t next;
	};

	struct ivlincomb
	{
		uint32_t* table;     /* Hash table. */
		ivlc_keyval_t* elts; /* List of elements indexed by table. */
		uint32_t card;       /* Number of (key,value) pairs. */
		uint32_t free_elts;  /* First free keyval item, 0 if none. */
		uint32_t elts_len;   /* Number of itmes used or on free_elts list. */
		uint32_t elts_sz;    /* Allocated items on elts. */
		uint32_t table_sz;   /* Allocated hash table size. */
	};

	/* Minimal number of table entries for each element. */
	constexpr uint32_t USE_FACTOR = 2;

	constexpr uint32_t INIT_TABLE_SIZE = 2003;

	constexpr uint32_t INIT_ELT_SIZE = 100;

	uint32_t ivlc_card(const ivlincomb* ht);

	ivlincomb* ivlc_new(uint32_t tabsz, uint32_t eltsz);

	void ivlc_free(ivlincomb* ht);

	void ivlc_reset(ivlincomb* ht);

	/* Return pointer to keyval_t, nullptr if key not in table. */
	ivlc_keyval_t* ivlc_lookup(const ivlincomb* ht, const ivector* key, uint32_t hash);

	/* Call only if key is not in table.  Insert key into table and return
	   a pointer to new value variable, nullptr if memory allocation
	   error. */
	ivlc_keyval_t* ivlc_insert(ivlincomb* ht, ivector* key, uint32_t hash, int32_t value);

	/* Return true if equal; ignore zero values. */
	bool ivlc_equals(const ivlincomb* ht1, const ivlincomb* ht2);

	void ivlc_print_stat(const ivlincomb* ht);

	struct ivlc_iter
	{
		const ivlincomb* ht;
		size_t index;
		size_t i;
	};

	bool ivlc_good(const ivlc_iter* itr);

	void ivlc_first(const ivlincomb* ht, ivlc_iter* itr);

	void ivlc_next(ivlc_iter* itr);

	ivlc_keyval_t* ivlc_keyval(const ivlc_iter* itr);

	void ivlc_free_all(ivlincomb* ht);

	constexpr int LC_COPY_KEY = 1;
	constexpr int LC_FREE_KEY = 0;

	constexpr int LC_FREE_ZERO = 2;

	// return true if secceeded
	bool ivlc_add_element(ivlincomb* ht, int32_t c, ivector* key, uint32_t hash, int opt);

	// return true if secceeded
	bool ivlc_add_multiple(ivlincomb* dst, int32_t c, ivlincomb* src, int opt);

	void ivlc_print(const ivlincomb* ht, int opt_zero);

	constexpr uint32_t IVLC_HASHTABLE_SZ = 2003;
	constexpr uint32_t IVLC_ARRAY_SZ = 100;
#ifdef __cplusplus
}
#endif

#endif
