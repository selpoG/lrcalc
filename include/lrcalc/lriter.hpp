#ifndef LRCALC_LRITER_H
#define LRCALC_LRITER_H

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	struct lrit_box
	{
		int value;
		int max;
		int above;
		int right;
	};

	struct lrtab_iter
	{
		ivector* cont;
		int size;
		int array_len;
		lrit_box* array;
	};

	lrtab_iter* lrit_new(const ivector& outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
	                     int partsz);

	void lrit_free(lrtab_iter* lrit);

	void lrit_print_skewtab(const lrtab_iter& lrit, const ivector& outer, const ivector* inner);

	bool lrit_good(const lrtab_iter& lrit);

	void lrit_next(lrtab_iter& lrit);
#ifdef __cplusplus
}
#endif

#endif
