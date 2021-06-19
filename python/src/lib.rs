use pyo3::{
    prelude::*,
    types::{IntoPyDict, PyDict, PyList, PyTuple},
    wrap_pyfunction,
};

use lrcalc_helper::{
    ivector::{iv_free_ptr, IntVector},
    ivlincomb::{ivlc_free_all, LinearCombination},
    lriter::{lrit_free, lrit_good, lrit_new, lrit_next, LRTableauIterator},
    part::{part_qdegree, part_qentry},
    schublib::{mult_schubert, mult_schubert_str, trans},
    schur::{schur_coprod, schur_lrcoef, schur_mult, schur_mult_fusion, schur_skew},
};

struct SafeIntVector {
    data: *mut IntVector,
    owned: bool,
}

impl From<Vec<i32>> for SafeIntVector {
    fn from(v: Vec<i32>) -> Self {
        SafeIntVector {
            data: IntVector::from_vec(v),
            owned: true,
        }
    }
}

impl SafeIntVector {
    pub fn deref(&self) -> &IntVector {
        unsafe { &*self.data }
    }
    pub fn deref_mut(&mut self) -> &mut IntVector {
        unsafe { &mut *self.data }
    }
}

impl Drop for SafeIntVector {
    fn drop(&mut self) {
        if self.owned && self.data != std::ptr::null_mut() {
            iv_free_ptr(self.data)
        }
    }
}

struct SafeLinearCombination {
    data: *mut LinearCombination,
    owned: bool,
}

impl SafeLinearCombination {
    pub fn new(p: *mut LinearCombination) -> SafeLinearCombination {
        SafeLinearCombination {
            data: p,
            owned: true,
        }
    }
    pub fn to_dict_of_vecs(self) -> Vec<(Vec<i32>, i32)> {
        let mut ans = Vec::new();
        for kv in self.deref().iter() {
            let key = unsafe { &*kv.key };
            ans.push(((&key[..]).to_vec(), kv.value))
        }
        ans
    }
    pub fn deref(&self) -> &LinearCombination {
        unsafe { &*self.data }
    }
}

fn to_py_dict_of_tuple(py: Python, vals: Vec<(Vec<i32>, i32)>) -> &PyDict {
    vals.into_iter()
        .map(|(k, v)| (PyTuple::new(py, k), v))
        .collect::<Vec<_>>()
        .into_py_dict(py)
}

fn part_len(v: &[i32]) -> usize {
    let mut l = v.len();
    while l > 0 && v[l - 1] == 0 {
        l -= 1
    }
    l
}

fn as_part(v: &[i32]) -> &[i32] {
    &v[..part_len(&v[..])]
}

fn to_py_dict_of_part(py: Python, vals: Vec<(Vec<i32>, i32)>) -> &PyDict {
    vals.into_iter()
        .map(|(k, v)| (PyTuple::new(py, as_part(&k[..])), v))
        .collect::<Vec<_>>()
        .into_py_dict(py)
}

fn as_quantum(v: &[i32], level: i32) -> (Vec<i32>, i32) {
    let d = part_qdegree(v, level);
    let mut n = v.len() as i32;
    while n > 0 && part_qentry(&v[..], n - 1, d, level) == 0 {
        n -= 1
    }
    (
        (0..n).map(|i| part_qentry(&v[..], i, d, level)).collect(),
        d,
    )
}

fn to_py_dict_of_quantum(
    py: Python,
    vals: Vec<(Vec<i32>, i32)>,
    level: i32,
    degrees: bool,
) -> &PyDict {
    vals.into_iter()
        .map(|(k, v)| {
            let q = as_quantum(&k[..], level);
            (
                if degrees {
                    PyTuple::new(py, [Py::from(PyTuple::new(py, q.0)), q.1.to_object(py)])
                } else {
                    PyTuple::new(py, q.0)
                },
                v,
            )
        })
        .collect::<Vec<_>>()
        .into_py_dict(py)
}

fn as_pair(v: &[i32], rows: i32, cols: i32) -> (Vec<i32>, Vec<i32>) {
    let mut a = Vec::with_capacity(rows as usize);
    for i in 0..(rows as usize) {
        if v[i] == cols {
            break;
        }
        a.push(v[i] - cols)
    }
    let mut b = Vec::with_capacity(rows as usize);
    for i in (rows as usize)..v.len() {
        if v[i] == 0 {
            break;
        }
        b.push(v[i])
    }
    (a, b)
}

fn to_py_dict_of_pair(py: Python, vals: Vec<(Vec<i32>, i32)>, rows: i32, cols: i32) -> &PyDict {
    vals.into_iter()
        .map(|(k, v)| {
            let q = as_pair(&k[..], rows, cols);
            (
                PyTuple::new(py, [PyTuple::new(py, q.0), PyTuple::new(py, q.1)]),
                v,
            )
        })
        .collect::<Vec<_>>()
        .into_py_dict(py)
}

impl Drop for SafeLinearCombination {
    fn drop(&mut self) {
        if self.owned && self.data != std::ptr::null_mut() {
            ivlc_free_all(self.data)
        }
    }
}

struct SafeLRTableauIterator {
    data: *mut LRTableauIterator,
    owned: bool,
}

impl SafeLRTableauIterator {
    pub fn new(p: *mut LRTableauIterator) -> SafeLRTableauIterator {
        SafeLRTableauIterator {
            data: p,
            owned: true,
        }
    }
    pub fn deref(&self) -> &LRTableauIterator {
        unsafe { &*self.data }
    }
    pub fn deref_mut(&mut self) -> &mut LRTableauIterator {
        unsafe { &mut *self.data }
    }
}

impl Drop for SafeLRTableauIterator {
    fn drop(&mut self) {
        if self.owned && self.data != std::ptr::null_mut() {
            lrit_free(self.data)
        }
    }
}

#[pyfunction]
#[text_signature = "(out, inn1, inn2)"]
/// Compute a single Littlewood-Richardson coefficient.
fn lrcoef(out: Vec<i32>, inn1: Vec<i32>, inn2: Vec<i32>) -> PyResult<i64> {
    let out = SafeIntVector::from(out);
    let inn1 = SafeIntVector::from(inn1);
    let inn2 = SafeIntVector::from(inn2);
    let ans = schur_lrcoef(out.deref(), inn1.deref(), inn2.deref());
    Ok(ans)
}

#[pyfunction(rows = "None", cols = "None")]
#[text_signature = "(sh1, sh2, rows=None, cols=None)"]
/// Compute the product of two Schur functions.
fn mult(
    py: Python,
    sh1: Vec<i32>,
    sh2: Vec<i32>,
    rows: Option<i32>,
    cols: Option<i32>,
) -> PyResult<Py<PyDict>> {
    let sh1 = SafeIntVector::from(sh1);
    let sh2 = SafeIntVector::from(sh2);
    let rows = rows.unwrap_or(-1);
    let cols = cols.unwrap_or(-1);
    let dict = to_py_dict_of_part(
        py,
        SafeLinearCombination::new(schur_mult(sh1.deref(), sh2.deref(), rows, cols, -1))
            .to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction]
#[text_signature = "(sh1, sh2, rows, level)"]
/// Compute a product in the fusion ring of type A.
fn mult_fusion(
    py: Python,
    sh1: Vec<i32>,
    sh2: Vec<i32>,
    rows: i32,
    level: i32,
) -> PyResult<Py<PyDict>> {
    let sh1 = SafeIntVector::from(sh1);
    let sh2 = SafeIntVector::from(sh2);
    let dict = to_py_dict_of_part(
        py,
        SafeLinearCombination::new(schur_mult_fusion(sh1.deref(), sh2.deref(), rows, level))
            .to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction(degrees = "false")]
#[text_signature = "(sh1, sh2, rows, cols, degrees=False)"]
/// Compute quantum product of Schubert classes on a Grassmannian.
fn mult_quantum(
    py: Python,
    sh1: Vec<i32>,
    sh2: Vec<i32>,
    rows: i32,
    cols: i32,
    degrees: bool,
) -> PyResult<Py<PyDict>> {
    let sh1 = SafeIntVector::from(sh1);
    let sh2 = SafeIntVector::from(sh2);
    let dict = to_py_dict_of_quantum(
        py,
        SafeLinearCombination::new(schur_mult_fusion(sh1.deref(), sh2.deref(), rows, cols))
            .to_dict_of_vecs(),
        cols,
        degrees,
    );
    Ok(Py::from(dict))
}

#[pyfunction(rows = "None")]
#[text_signature = "(outer, inner, rows=None)"]
/// Compute the Schur expansion of a skew Schur function.
fn skew(py: Python, outer: Vec<i32>, inner: Vec<i32>, rows: Option<i32>) -> PyResult<Py<PyDict>> {
    let outer = SafeIntVector::from(outer);
    let inner = SafeIntVector::from(inner);
    let rows = rows.unwrap_or(-1);
    let dict = to_py_dict_of_part(
        py,
        SafeLinearCombination::new(schur_skew(outer.deref(), inner.deref(), rows, -1))
            .to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction(all = "false")]
#[text_signature = "(sh, all=False)"]
/// Compute the coproduct of a Schur function.
fn coprod(py: Python, sh: Vec<i32>, all: bool) -> PyResult<Py<PyDict>> {
    let sh = SafeIntVector::from(sh);
    let mut row = sh.deref().length as usize;
    while row > 0 && sh.deref()[row - 1] == 0 {
        row -= 1
    }
    let col = if row == 0 { 0 } else { sh.deref()[0] };
    let dict = to_py_dict_of_pair(
        py,
        SafeLinearCombination::new(schur_coprod(sh.deref(), row as i32, col, -1, all))
            .to_dict_of_vecs(),
        row as i32,
        col,
    );
    Ok(Py::from(dict))
}

#[pyfunction]
#[text_signature = "(w)"]
/// Compute the Schubert polynomial of a permutation.
fn schubert_poly(py: Python, w: Vec<i32>) -> PyResult<Py<PyDict>> {
    let w = SafeIntVector::from(w);
    let dict = to_py_dict_of_tuple(
        py,
        SafeLinearCombination::new(trans(&w.deref()[..], 0)).to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction(rank = "0")]
#[text_signature = "(w1, w2, rank=0)"]
/// Compute the product of two Schubert polynomials.
fn schubmult(py: Python, w1: Vec<i32>, w2: Vec<i32>, rank: i32) -> PyResult<Py<PyDict>> {
    let mut w1 = SafeIntVector::from(w1);
    let mut w2 = SafeIntVector::from(w2);
    let dict = to_py_dict_of_tuple(
        py,
        SafeLinearCombination::new(mult_schubert(w1.deref_mut(), w2.deref_mut(), rank))
            .to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction]
#[text_signature = "(str1, str2)"]
/// Compute product of Schubert polynomials using string notation.
fn schubmult_str(py: Python, str1: Vec<i32>, str2: Vec<i32>) -> PyResult<Py<PyDict>> {
    let mut str1 = SafeIntVector::from(str1);
    let mut str2 = SafeIntVector::from(str2);
    let dict = to_py_dict_of_tuple(
        py,
        SafeLinearCombination::new(mult_schubert_str(str1.deref_mut(), str2.deref_mut()))
            .to_dict_of_vecs(),
    );
    Ok(Py::from(dict))
}

#[pyfunction(rows = "None")]
#[text_signature = "(outer, inner, rows=None)"]
/// Iterate through column words of LR tableaux of given skew shape.
fn lr_iterator(
    py: Python,
    outer: Vec<i32>,
    inner: Vec<i32>,
    rows: Option<i32>,
) -> PyResult<Py<PyList>> {
    let outer = SafeIntVector::from(outer);
    let inner = SafeIntVector::from(inner);
    let rows = rows.unwrap_or(-1);
    let mut it = SafeLRTableauIterator::new(lrit_new(
        outer.deref(),
        inner.deref(),
        std::ptr::null(),
        rows,
        -1,
        -1,
    ));
    let mut ans: Vec<Vec<i32>> = Vec::new();
    while lrit_good(it.deref()) {
        let array =
            unsafe { std::slice::from_raw_parts(it.deref().array, it.deref().size as usize) };
        ans.push(array.iter().map(|b| b.value).collect());
        lrit_next(it.deref_mut());
    }
    Ok(Py::from(PyList::new(
        py,
        ans.into_iter().map(|w| PyTuple::new(py, w)),
    )))
}

#[pymodule]
/// Python bindings for the Littlewood-Richardson Calculator.
fn lrcalc(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(lrcoef, m)?)?;
    m.add_function(wrap_pyfunction!(mult, m)?)?;
    m.add_function(wrap_pyfunction!(mult_fusion, m)?)?;
    m.add_function(wrap_pyfunction!(mult_quantum, m)?)?;
    m.add_function(wrap_pyfunction!(skew, m)?)?;
    m.add_function(wrap_pyfunction!(coprod, m)?)?;
    m.add_function(wrap_pyfunction!(schubert_poly, m)?)?;
    m.add_function(wrap_pyfunction!(schubmult, m)?)?;
    m.add_function(wrap_pyfunction!(schubmult_str, m)?)?;
    m.add_function(wrap_pyfunction!(lr_iterator, m)?)?;

    Ok(())
}
