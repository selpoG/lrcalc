from libc.stdint cimport uint32_t, int32_t

cdef extern from "lrcalc/ivector.h":
    ctypedef struct ivector:
        uint32_t length
        int32_t array[1]

    ivector *iv_new(uint32_t length)
    void iv_free(ivector *v)


cdef extern from "lrcalc/part.h":
    int part_qdegree(ivector *p, int level)
    int part_qentry(ivector *p, int i, int d, int level)


cdef extern from "lrcalc/ivlincomb.h":
    ctypedef struct ivlincomb:
        pass

    void ivlc_free_all(ivlincomb *lc)

    ctypedef struct ivlc_iter:
        ivlincomb *ht
        uint32_t index
        uint32_t i

    void ivlc_first(ivlincomb *lc, ivlc_iter *itr)
    bint ivlc_good(ivlc_iter *itr)
    void ivlc_next(ivlc_iter *itr)
    ivector *ivlc_key(ivlc_iter *itr)
    int32_t ivlc_value(ivlc_iter *itr)


cdef extern from "lrcalc/schur.h":
    ivlincomb *schur_mult(
        ivector *sh1, ivector *sh2, int rows, int cols, int partsz)
    ivlincomb *schur_mult_fusion(
        ivector *sh1, ivector *sh2, int rows, int level)
    ivlincomb *schur_skew(
        ivector *outer, ivector *inner, int rows, int partsz)
    ivlincomb *schur_coprod(
        ivector *sh, int rows, int cols, int partsz, int all)

    long long schur_lrcoef(ivector *outer, ivector *inner1, ivector *inner2)


cdef extern from "lrcalc/schublib.h":
    ivlincomb *trans(ivector *w, int vars)
    ivlincomb *monk(int i, ivlincomb *slc, int rank)
    ivlincomb *mult_schubert(ivector *w1, ivector *w2, int rank)
    ivlincomb *mult_schubert_str(ivector *str1, ivector *str2)


cdef extern from "lrcalc/lriter.h":
    ctypedef struct lrit_box:
        int value
        int max
        int above
        int right

    ctypedef struct lrtab_iter:
        ivector *cont
        int size
        int array_len
        lrit_box array[1]

    lrtab_iter *lrit_new(ivector *outer, ivector *inner, ivector *content,
                         int maxrows, int maxcols, int partsz)
    int lrit_good(lrtab_iter *lrit)
    void lrit_next(lrtab_iter *lrit)
    void lrit_free(lrtab_iter *lrit)
