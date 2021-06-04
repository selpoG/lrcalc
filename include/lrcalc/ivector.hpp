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

	ivector* into_iv(const int32_t* p, uint32_t length);

	void iv_free(ivector* v);

	uint32_t iv_hash(const ivector& v);

	int32_t iv_sum(const ivector& v);

	void iv_print(const ivector& v);
	void iv_printnl(const ivector& v);

#define iv_length(v) ((v)->length)

#define iv_elem(v, i) ((v)->array[i])

#ifdef __cplusplus
}
#endif

#endif
