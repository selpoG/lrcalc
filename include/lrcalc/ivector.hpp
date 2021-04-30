#ifndef LRCALC_IVECTOR_H
#define LRCALC_IVECTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	struct ivector
	{
		uint32_t length;
		int32_t* array;
	};

	ivector* iv_new(uint32_t length);

	ivector* iv_new_zero(uint32_t length);

	void iv_free(ivector* v);

	ivector* iv_new_copy(const ivector* v);

	void iv_set_zero(ivector* v);

	void iv_copy(ivector* d, const ivector* s);

	int iv_cmp(const ivector* v1, const ivector* v2);

	uint32_t iv_hash(const ivector* v);

	int32_t iv_sum(const ivector* v);
	int iv_lesseq(const ivector* v1, const ivector* v2);
	void iv_mult(ivector* dst, int32_t c, const ivector* src);
	void iv_div(ivector* dst, const ivector* src, int32_t c);
	int32_t iv_max(const ivector* v);
	int32_t iv_min(const ivector* v);
	void iv_reverse(ivector* dst, const ivector* src);

	int32_t iv_gcd(const ivector* v);

	void iv_print(const ivector* v);
	void iv_printnl(const ivector* v);

#define iv_length(v) ((v)->length)

#define iv_elem(v, i) ((v)->array[i])

#ifdef __cplusplus
}
#endif

#endif
