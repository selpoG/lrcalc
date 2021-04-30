#ifndef LRCALC_MAPLE_H
#define LRCALC_MAPLE_H

#ifdef __cplusplus
extern "C"
{
#endif
	void maple_print_lincomb(const ivlincomb* ht, const char* letter, int nz);

	void maple_qprint_lincomb(const ivlincomb* lc, int level, const char* letter);
#ifdef __cplusplus
}
#endif

#endif
