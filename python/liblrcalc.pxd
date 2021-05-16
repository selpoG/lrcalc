from libc.stdint cimport int32_t, uint32_t


cdef extern from "lrcalc/ivector.hpp":
    ctypedef struct ivector:
        uint32_t length
        int32_t *array

    ivector *iv_new(uint32_t length)
    void iv_free(ivector *v)


cdef extern from "lrcalc/part.hpp":
    int part_qdegree(const ivector *p, int level)
    int part_qentry(const ivector *p, int i, int d, int level)


cdef extern from "lrcalc/ivlincomb.hpp":
    ctypedef struct ivlc_keyval_t:
        ivector *key
        int32_t value
        uint32_t hash
        uint32_t next

    ctypedef struct ivlincomb:
        uint32_t *table
        ivlc_keyval_t *elts
        uint32_t card
        uint32_t free_elts
        uint32_t elts_len
        uint32_t elts_sz
        uint32_t table_sz

    void ivlc_free_all(ivlincomb *lc)

    ctypedef struct ivlc_iter:
        ivlincomb *ht
        size_t index
        size_t i

    void ivlc_first(const ivlincomb *lc, ivlc_iter *itr)
    bint ivlc_good(const ivlc_iter *itr)
    void ivlc_next(ivlc_iter *itr)
    ivlc_keyval_t *ivlc_keyval(const ivlc_iter *itr)


cdef extern from "lrcalc/schur.hpp":
    ivlincomb *schur_mult(
        const ivector *sh1, const ivector *sh2, int rows, int cols, int partsz)
    ivlincomb *schur_mult_fusion(
        const ivector *sh1, const ivector *sh2, int rows, int level)
    ivlincomb *schur_skew(
        const ivector *outer, const ivector *inner, int rows, int partsz)
    ivlincomb *schur_coprod(
        const ivector *sh, int rows, int cols, int partsz, bint all)

    long long schur_lrcoef(const ivector *outer, const ivector *inner1, const ivector *inner2)


cdef extern from "lrcalc/schublib.hpp":
    ivlincomb *trans(ivector *w, int vars)
    ivlincomb *monk(int i, const ivlincomb *slc, int rank)
    ivlincomb *mult_schubert(ivector *w1, ivector *w2, int rank)
    ivlincomb *mult_schubert_str(const ivector *str1, const ivector *str2)


cdef extern from "lrcalc/lriter.hpp":
    ctypedef struct lrit_box:
        int value
        int max
        int above
        int right

    ctypedef struct lrtab_iter:
        ivector *cont
        int size
        int array_len
        lrit_box *array

    lrtab_iter *lrit_new(const ivector *outer, const ivector *inner, const ivector *content,
                         int maxrows, int maxcols, int partsz)
    bint lrit_good(const lrtab_iter *lrit)
    void lrit_next(lrtab_iter *lrit)
    void lrit_free(lrtab_iter *lrit)
