"""Python bindings for the Littlewood-Richardson Calculator."""


from liblrcalc cimport *

cdef ivector *iv_newpy(pv):
    cdef ivector *v
    cdef int i
    v = iv_new(len(pv))
    if v is NULL:
        raise MemoryError()
    for i in range(len(pv)):
        v.array[i] = pv[i]
    return v


cdef tuple iv_tuple(ivector *v):
    cdef int i
    return tuple(v.array[i] for i in range(v.length))

cdef dict ivlc_dict_tuple(ivlincomb *lc):
    cdef ivlc_iter itr
    res = dict()
    ivlc_first(lc, &itr)
    while ivlc_good(&itr):
        res[iv_tuple(ivlc_key(&itr))] = ivlc_value(&itr)
        ivlc_next(&itr)
    return res


cdef tuple iv_part(ivector *v):
    cdef int i, n
    n = v.length
    while n > 0 and v.array[n-1] == 0:
        n -= 1
    return tuple(v.array[i] for i in range(n))

cdef dict ivlc_dict_part(ivlincomb *lc):
    cdef ivlc_iter itr
    res = dict()
    ivlc_first(lc, &itr)
    while ivlc_good(&itr):
        res[iv_part(ivlc_key(&itr))] = ivlc_value(&itr)
        ivlc_next(&itr)
    return res


cdef tuple iv_quantum(ivector *v, int level, bint degrees):
    cdef int i, d, n
    cdef tuple p
    d = part_qdegree(v, level)
    n = v.length
    while n > 0 and part_qentry(v, n-1, d, level) == 0:
        n -= 1
    p = tuple(part_qentry(v, i, d, level) for i in range(n))
    return (p, d) if degrees else p

cdef dict ivlc_dict_quantum(ivlincomb *lc, int level, bint degrees):
    cdef ivlc_iter itr
    res = dict()
    ivlc_first(lc, &itr)
    while ivlc_good(&itr):
        res[iv_quantum(ivlc_key(&itr), level, degrees)] = ivlc_value(&itr)
        ivlc_next(&itr)
    return res


cdef tuple iv_pair(ivector *v, int rows, int cols):
    cdef int i;
    p1 = tuple(v.array[i] - cols for i in range(rows) if v.array[i] != cols)
    p2 = tuple(v.array[i] for i in range(rows, v.length) if v.array[i] != 0)
    return (p1, p2)

cdef dict ivlc_dict_pair(ivlincomb *lc, int rows, int cols):
    cdef ivlc_iter itr
    res = dict()
    ivlc_first(lc, &itr)
    while ivlc_good(&itr):
        res[iv_pair(ivlc_key(&itr), rows, cols)] = ivlc_value(&itr)
        ivlc_next(&itr)
    return res


def lrcoef(out, inn1, inn2):
    """Compute a single Littlewood-Richardson coefficient."""

    cdef ivector *cout = NULL
    cdef ivector *cinn1 = NULL
    cdef ivector *cinn2 = NULL
    try:
        cout = iv_newpy(out)
        cinn1 = iv_newpy(inn1)
        cinn2 = iv_newpy(inn2)
        return schur_lrcoef(cout, cinn1, cinn2)
    finally:
        if cinn2 is not NULL:
            iv_free(cinn2)
        if cinn1 is not NULL:
            iv_free(cinn1)
        if cout is not NULL:
            iv_free(cout)


def mult(sh1, sh2, int rows=-1, int cols=-1):
    """Compute the product of two Schur functions."""

    cdef ivector *csh1 = NULL
    cdef ivector *csh2 = NULL
    cdef ivlincomb *cprd = NULL
    try:
        csh1 = iv_newpy(sh1)
        csh2 = iv_newpy(sh2)
        cprd = schur_mult(csh1, csh2, rows, cols, -1)
        if cprd is NULL:
            raise MemoryError()
        return ivlc_dict_part(cprd)
    finally:
        if cprd is not NULL:
            ivlc_free_all(cprd)
        if csh2 is not NULL:
            iv_free(csh2)
        if csh1 is not NULL:
            iv_free(csh1)


def mult_fusion(sh1, sh2, int rows, int level):
    """Compute a product in the fusion ring of type A."""

    cdef ivector *csh1 = NULL
    cdef ivector *csh2 = NULL
    cdef ivlincomb *cprd = NULL
    try:
        csh1 = iv_newpy(sh1)
        csh2 = iv_newpy(sh2)
        cprd = schur_mult_fusion(csh1, csh2, rows, level)
        if cprd is NULL:
            raise MemoryError()
        return ivlc_dict_part(cprd)
    finally:
        if cprd is not NULL:
            ivlc_free_all(cprd)
        if csh2 is not NULL:
            iv_free(csh2)
        if csh1 is not NULL:
            iv_free(csh1)


def mult_quantum(sh1, sh2, int rows, int cols, bint degrees=False):
    """Compute quantum product of Schubert classes on a Grassmannian."""

    cdef ivector *csh1 = NULL
    cdef ivector *csh2 = NULL
    cdef ivlincomb *cprd = NULL
    try:
        csh1 = iv_newpy(sh1)
        csh2 = iv_newpy(sh2)
        cprd = schur_mult_fusion(csh1, csh2, rows, cols)
        if cprd is NULL:
            raise MemoryError()
        return ivlc_dict_quantum(cprd, cols, degrees)
    finally:
        if cprd is not NULL:
            ivlc_free_all(cprd)
        if csh2 is not NULL:
            iv_free(csh2)
        if csh1 is not NULL:
            iv_free(csh1)


def skew(outer, inner, int rows=-1):
    """Compute the Schur expansion of a skew Schur function."""

    cdef ivector *cout = NULL
    cdef ivector *cinn = NULL
    cdef ivlincomb *cres = NULL
    try:
        cout = iv_newpy(outer)
        cinn = iv_newpy(inner)
        cres = schur_skew(cout, cinn, rows, -1)
        if cres is NULL:
            raise MemoryError()
        return ivlc_dict_part(cres)
    finally:
        if cres is not NULL:
            ivlc_free_all(cres)
        if cinn is not NULL:
            iv_free(cinn)
        if cout is not NULL:
            iv_free(cout)


def coprod(sh, bint all=False):
    """Compute the coproduct of a Schur function."""

    cdef ivector *csh = NULL
    cdef ivlincomb *cres = NULL
    cdef int rows, cols
    try:
        csh = iv_newpy(sh)
        rows = csh.length
        while rows > 0 and csh.array[rows - 1] == 0:
            rows -= 1
        cols = 0 if rows == 0 else csh.array[0]
        cres = schur_coprod(csh, rows, cols, -1, all)
        if cres is NULL:
            raise MemoryError()
        return ivlc_dict_pair(cres, rows, cols)
    finally:
        if cres is not NULL:
            ivlc_free_all(cres)
        if csh is not NULL:
            iv_free(csh)


def schubert_poly(w):
    """Compute the Schubert polynomial of a permutation."""

    cdef ivector *cw = NULL
    cdef ivlincomb *cres = NULL
    try:
        cw = iv_newpy(w)
        cres = trans(cw, 0)
        if cres is NULL:
            raise MemoryError()
        return ivlc_dict_tuple(cres)
    finally:
        if cres is not NULL:
            ivlc_free_all(cres)
        if cw is not NULL:
            iv_free(cw)


def schubmult(w1, w2, int rank=0):
    """Compute the product of two Schubert polynomials."""

    cdef ivector *cw1 = NULL
    cdef ivector *cw2 = NULL
    cdef ivlincomb *cres = NULL
    try:
        cw1 = iv_newpy(w1)
        cw2 = iv_newpy(w2)
        cres = mult_schubert(cw1, cw2, rank)
        if cres is NULL:
            raise MemoryError()
        return ivlc_dict_tuple(cres)
    finally:
        if cres is not NULL:
            ivlc_free_all(cres)
        if cw2 is not NULL:
            iv_free(cw2)
        if cw1 is not NULL:
            iv_free(cw1)


def schubmult_str(str1, str2):
    """Compute product of Schubert polynomials using string notation."""

    cdef ivector *cs1 = NULL
    cdef ivector *cs2 = NULL
    cdef ivlincomb *cres = NULL
    try:
        cs1 = iv_newpy(str1)
        cs2 = iv_newpy(str2)
        cres = mult_schubert_str(cs1, cs2)
        if cres is NULL:
            raise MemoryError()
        return ivlc_dict_tuple(cres)
    finally:
        if cres is not NULL:
            ivlc_free_all(cres)
        if cs2 is not NULL:
            iv_free(cs2)
        if cs1 is not NULL:
            iv_free(cs1)


cdef class lr_iterator:
    """Iterate through column words of LR tableaux of given skew shape."""

    cdef lrtab_iter *_itr

    def __cinit__(self, outer, inner, int rows=-1):
        cdef ivector *out = NULL
        cdef ivector *inn = NULL
        try:
            out = iv_newpy(outer)
            inn = iv_newpy(inner)
            self._itr = lrit_new(out, inn, NULL, rows, -1, -1)
            if self._itr is NULL:
                raise MemoryError()
        finally:
            if inn is not NULL:
                iv_free(inn)
            if out is not NULL:
                iv_free(out)

    def __iter__(self):
        return self

    def __next__(self):
        cdef int i
        if not lrit_good(self._itr):
            raise StopIteration
        word = tuple(self._itr.array[i].value
                     for i in range(self._itr.size))
        lrit_next(self._itr)
        return word

    def __dealloc(self):
        if self._itr is not NULL:
            lrit_free(self._itr)