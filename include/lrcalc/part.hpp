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

	bool pitr_good(const part_iter& itr);

	void pitr_first(part_iter& itr, ivector& p, int rows, int cols, const ivector* outer, const ivector* inner,
	                int size, int opt);

	void pitr_next(part_iter& itr);
#ifdef __cplusplus
}
#endif

#endif
