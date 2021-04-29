#ifndef LRCALC_LRITER_H
#define LRCALC_LRITER_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef LRCALC_LRITER_C
#undef INLINE
#define INLINE CINLINE
#endif

typedef struct
{
	int value;
	int max;
	int above;
	int right;
} lrit_box;

typedef struct
{
	ivector* cont;
	int size;
	int array_len;
	lrit_box array[];
} lrtab_iter;

lrtab_iter* lrit_new(const ivector* outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
                     int partsz);

INLINE void lrit_free(lrtab_iter* lrit)
{
	iv_free(lrit->cont);
	ml_free(lrit);
}

void lrit_print_skewtab(const lrtab_iter* lrit, const ivector* outer, const ivector* inner);
void lrit_dump(const lrtab_iter* lrit);
void lrit_dump_skew(const lrtab_iter* lrit, const ivector* outer, const ivector* inner);

INLINE int lrit_good(const lrtab_iter* lrit) { return lrit->size >= 0; }

INLINE void lrit_next(lrtab_iter* lrit)
{
	ivector* cont = lrit->cont;
	lrit_box* array = lrit->array;
	int size = lrit->size;
	lrit_box* box_bound = array + size;
	lrit_box* box;
	for (box = array; box != box_bound; box++)
	{
		int max = array[box->right].value;
		if (max > box->max) max = box->max;
		int x = box->value;
		iv_elem(cont, x)--;
		x++;
		while (x <= max && iv_elem(cont, x) == iv_elem(cont, x - 1)) x++;
		if (x > max) continue;

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

INLINE ivlincomb* lrit_count(lrtab_iter* lrit)
{
	ivector* cont = lrit->cont;
	ivlincomb* lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
	if (lc == nullptr) return nullptr;
	for (; lrit_good(lrit); lrit_next(lrit))
		if (ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY) != 0)
		{
			ivlc_free_all(lc);
			return nullptr;
		}
	return lc;
}

ivlincomb* lrit_expand(const ivector* outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
                       int partsz);

#endif
