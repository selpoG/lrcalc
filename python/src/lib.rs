use pyo3::{
    prelude::*,
    types::{IntoPyDict, PyDict, PyList, PyTuple},
    wrap_pyfunction,
};

use lrcalc::helper::{
    ivlincomb::LinearCombination,
    part::{part_qdegree, part_qentry},
    schublib::{mult_schubert, mult_schubert_str, trans},
    schur::{schur_coprod, schur_lrcoef, schur_mult, schur_mult_fusion, schur_skew},
};
use lrcalc::lriter::{lrit_good, lrit_new, lrit_next};

pub fn ivlc_to_dict_of_vecs(lc: LinearCombination) -> Vec<(Vec<i32>, i32)> {
    let mut ans = Vec::new();
    for (k, &v) in lc.map.iter() {
        ans.push(((&k.ptr[..]).to_vec(), v))
    }
    ans
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
    &v[..part_len(v)]
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
    while n > 0 && part_qentry(v, n - 1, d, level) == 0 {
        n -= 1
    }
    ((0..n).map(|i| part_qentry(v, i, d, level)).collect(), d)
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
    for &x in v.iter().take(rows as usize) {
        if x == cols {
            break;
        }
        a.push(x - cols)
    }
    let mut b = Vec::with_capacity(rows as usize);
    for &x in v.iter().skip(rows as usize) {
        if x == 0 {
            break;
        }
        b.push(x)
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

#[pyfunction]
#[text_signature = "(out, inn1, inn2)"]
/// Compute a single Littlewood-Richardson coefficient.
fn lrcoef(out: Vec<i32>, inn1: Vec<i32>, inn2: Vec<i32>) -> PyResult<i64> {
    let ans = schur_lrcoef(&out.into(), &inn1.into(), &inn2.into());
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
    let rows = rows.unwrap_or(-1);
    let cols = cols.unwrap_or(-1);
    let dict = to_py_dict_of_part(
        py,
        ivlc_to_dict_of_vecs(schur_mult(&sh1.into(), &sh2.into(), rows, cols, -1)),
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
    let dict = to_py_dict_of_part(
        py,
        ivlc_to_dict_of_vecs(schur_mult_fusion(&sh1.into(), &sh2.into(), rows, level)),
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
    let dict = to_py_dict_of_quantum(
        py,
        ivlc_to_dict_of_vecs(schur_mult_fusion(&sh1.into(), &sh2.into(), rows, cols)),
        cols,
        degrees,
    );
    Ok(Py::from(dict))
}

#[pyfunction(rows = "None")]
#[text_signature = "(outer, inner, rows=None)"]
/// Compute the Schur expansion of a skew Schur function.
fn skew(py: Python, outer: Vec<i32>, inner: Vec<i32>, rows: Option<i32>) -> PyResult<Py<PyDict>> {
    let rows = rows.unwrap_or(-1);
    let dict = to_py_dict_of_part(
        py,
        ivlc_to_dict_of_vecs(schur_skew(&outer.into(), &inner.into(), rows, -1)),
    );
    Ok(Py::from(dict))
}

#[pyfunction(all = "false")]
#[text_signature = "(sh, all=False)"]
/// Compute the coproduct of a Schur function.
fn coprod(py: Python, sh: Vec<i32>, all: bool) -> PyResult<Py<PyDict>> {
    let mut row = sh.len();
    while row > 0 && sh[row - 1] == 0 {
        row -= 1
    }
    let col = if row == 0 { 0 } else { sh[0] };
    let dict = to_py_dict_of_pair(
        py,
        ivlc_to_dict_of_vecs(schur_coprod(&sh.into(), row as i32, col, -1, all)),
        row as i32,
        col,
    );
    Ok(Py::from(dict))
}

#[pyfunction]
#[text_signature = "(w)"]
/// Compute the Schubert polynomial of a permutation.
fn schubert_poly(py: Python, w: Vec<i32>) -> PyResult<Py<PyDict>> {
    let dict = to_py_dict_of_tuple(py, ivlc_to_dict_of_vecs(trans(&w[..], 0)));
    Ok(Py::from(dict))
}

#[pyfunction(rank = "0")]
#[text_signature = "(w1, w2, rank=0)"]
/// Compute the product of two Schubert polynomials.
fn schubmult(py: Python, w1: Vec<i32>, w2: Vec<i32>, rank: i32) -> PyResult<Py<PyDict>> {
    let dict = to_py_dict_of_tuple(
        py,
        ivlc_to_dict_of_vecs(mult_schubert(&mut w1.into(), &mut w2.into(), rank)),
    );
    Ok(Py::from(dict))
}

#[pyfunction]
#[text_signature = "(str1, str2)"]
/// Compute product of Schubert polynomials using string notation.
fn schubmult_str(py: Python, str1: Vec<i32>, str2: Vec<i32>) -> PyResult<Py<PyDict>> {
    let dict = to_py_dict_of_tuple(
        py,
        ivlc_to_dict_of_vecs(mult_schubert_str(&str1.into(), &str2.into()).unwrap()),
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
    let rows = rows.unwrap_or(-1);
    let mut it = lrit_new(&outer.into(), Some(&inner.into()), None, rows, -1, -1);
    let mut ans: Vec<Vec<i32>> = Vec::new();
    while lrit_good(&it) {
        let array = &it.array[0..it.size as usize];
        ans.push(array.iter().map(|b| b.value).collect());
        lrit_next(&mut it);
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
