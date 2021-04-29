#ifndef LRCALC_LRITER_H
#define LRCALC_LRITER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

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

	void lrit_free(lrtab_iter* lrit);

	void lrit_print_skewtab(const lrtab_iter* lrit, const ivector* outer, const ivector* inner);
	void lrit_dump(const lrtab_iter* lrit);
	void lrit_dump_skew(const lrtab_iter* lrit, const ivector* outer, const ivector* inner);

	int lrit_good(const lrtab_iter* lrit);

	void lrit_next(lrtab_iter* lrit);

	ivlincomb* lrit_count(lrtab_iter* lrit);

	ivlincomb* lrit_expand(const ivector* outer, const ivector* inner, const ivector* content, int maxrows, int maxcols,
	                       int partsz);

#ifdef __cplusplus
}
#endif
#endif
