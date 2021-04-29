#ifndef LRCALC_PART_H
#define LRCALC_PART_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lrcalc/alloc.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

	int part_valid(const ivector* p);

	int part_decr(const ivector* p);

	uint32_t part_length(const ivector* p);

	int part_entry(const ivector* p, int i);

	void part_chop(ivector* p);

	/* Must have len >= iv_length(p) and p was allocated with enough space. */
	void part_unchop(ivector* p, int len_);

	int part_leq(const ivector* p1, const ivector* p2);

	ivector* part_conj(const ivector* p);

	void part_print(const ivector* p);
	void part_printnl(const ivector* p);

	void part_print_lincomb(const ivlincomb* lc);

	/* Translate fusion algebra partitions to quantum cohomology notation. */

	int part_qdegree(const ivector* p, int level);

	int part_qentry(const ivector* p, int i, int d, int level);

	void part_qprint(const ivector* p, int level);
	void part_qprintnl(const ivector* p, int level);

	void part_qprint_lincomb(const ivlincomb* lc, int level);

	/* General partition iterator that the compiler will optimize when opt
	 * is known at compile time.
	 */

	typedef struct
	{
		ivector* part;
		const ivector* outer;
		const ivector* inner;
		int length;
		int rows;
		int opt;
	} part_iter;

#define PITR_USE_OUTER 1
#define PITR_USE_INNER 2
#define PITR_USE_SIZE 4

	int pitr_good(const part_iter* itr);

	int pitr_first(part_iter* itr, ivector* p, int rows, int cols, const ivector* outer, const ivector* inner, int size,
	               int opt);

	/* int pitr_first(part_iter *itr, ivector *p, int rows, int cols,
	 *                const ivector *outer, const ivector *inner, int size, int opt)
	 */

	void pitr_box_first(part_iter* itr, ivector* p, int rows, int cols);

	void pitr_box_sz_first(part_iter* itr, ivector* p, int rows, int cols, int size);

	void pitr_sub_first(part_iter* itr, ivector* p, const ivector* outer);

	void pitr_sub_sz_first(part_iter* itr, ivector* p, const ivector* outer, int size);

	void pitr_between_first(part_iter* itr, ivector* p, const ivector* outer, const ivector* inner);

	void pitr_between_sz_first(part_iter* itr, ivector* p, const ivector* outer, const ivector* inner, int size);

	void pitr_next(part_iter* itr);

#ifdef __cplusplus
}
#endif
#endif
