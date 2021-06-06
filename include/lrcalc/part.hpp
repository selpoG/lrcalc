#ifndef LRCALC_PART_H
#define LRCALC_PART_H

#include <stdint.h>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	bool part_valid(const ivector& p);

	uint32_t part_length(const ivector& p);

	int part_entry(const ivector& p, int i);

	/* Must have len >= iv_length(p) and p was allocated with enough space. */
	void part_unchop(ivector& p, int len_);

	bool part_leq(const ivector& p1, const ivector& p2);

	void part_print_lincomb(const ivlincomb& lc);

	/* Translate fusion algebra partitions to quantum cohomology notation. */

	int part_qdegree(const ivector& p, int level);
	int part_qentry(const ivector& p, int i, int d, int level);
	void part_qprint_lincomb(const ivlincomb& lc, int level);

	/* General partition iterator that the compiler will optimize when opt
	 * is known at compile time.
	 */

	struct part_iter
	{
		ivector* part;
		const ivector* outer;
		const ivector* inner;
		int length;
		int rows;
		int opt;
	};

	constexpr int PITR_USE_OUTER = 1;
	constexpr int PITR_USE_INNER = 2;
	constexpr int PITR_USE_SIZE = 4;

	bool pitr_good(const part_iter& itr);

	void pitr_first(part_iter& itr, ivector& p, int rows, int cols, const ivector* outer, const ivector* inner,
	                int size, int opt);

	/* void pitr_first(part_iter &itr, ivector &p, int rows, int cols,
	 *                 const ivector *outer, const ivector *inner, int size, int opt)
	 */

	void pitr_next(part_iter& itr);
#ifdef __cplusplus
}
#endif

#endif
