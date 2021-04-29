#ifndef LRCALC_SCHUR_H
#define LRCALC_SCHUR_H
#ifdef __cplusplus
extern "C"
{
#endif

	ivlincomb* schur_mult(const ivector* sh1, const ivector* sh2, int rows, int cols, int partsz);

	int fusion_reduce(ivector* la, int level, ivector* tmp);
	int fusion_reduce_lc(ivlincomb* lc, int level);
	ivlincomb* schur_mult_fusion(ivector* sh1, ivector* sh2, int rows, int level);

	ivlincomb* schur_skew(const ivector* outer, const ivector* inner, int rows, int partsz);

	ivlincomb* schur_coprod(const ivector* sh, int rows, int cols, int partsz, int all);

	long long schur_lrcoef(const ivector* outer, const ivector* inner1, const ivector* inner2);

#ifdef __cplusplus
}
#endif
#endif
