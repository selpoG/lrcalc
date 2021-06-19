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
	struct ivlc_keyval_t;

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

	struct ivlc_iter
	{
		const ivlincomb* ht;
		size_t index;
		size_t i;
		bool initialized;
	};

	void ivlc_free_all(ivlincomb* ht);

	void ivlc_print(const ivlincomb& ht);

	void ivlc_print_coprod(const ivlincomb& ht, uint32_t rows, int cols);
#ifdef __cplusplus
}
#endif

#endif
