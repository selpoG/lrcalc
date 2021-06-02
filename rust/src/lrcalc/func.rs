use lrcalc_helper::{
    part::{pitr_first_rs, pitr_good, pitr_next},
    perm::{all_perms as _all_perms, all_strings as _all_strings},
    schublib::{mult_poly_schubert, mult_schubert, mult_schubert_str, trans},
    schur::{
        fusion_reduce_lc, schur_coprod, schur_lrcoef, schur_mult, schur_mult_fusion, schur_skew,
    },
};

use super::ivector::IntVector;
use super::ivlist::VectorList;
use super::lincomb::LinearCombination;
use super::lriter::LRTableauIterator;

pub(crate) fn _schur_lrcoef(outer: &IntVector, inner1: &IntVector, inner2: &IntVector) -> i64 {
    schur_lrcoef(outer.data, inner1.data, inner2.data)
}

pub(crate) fn _mult_poly_schubert(
    mut poly: LinearCombination,
    perm: &IntVector,
    rank: ::std::os::raw::c_int,
) -> LinearCombination {
    let ans = mult_poly_schubert(poly.data, perm.data, rank);
    poly.owned = false;
    ans.into()
}

pub(crate) fn _schur_mult(
    sh1: &IntVector,
    sh2: &IntVector,
    rows: ::std::os::raw::c_int,
    cols: ::std::os::raw::c_int,
    partsz: ::std::os::raw::c_int,
) -> LinearCombination {
    schur_mult(sh1.data, sh2.data, rows, cols, partsz).into()
}

pub(crate) fn _schur_mult_fusion(
    sh1: &IntVector,
    sh2: &IntVector,
    rows: ::std::os::raw::c_int,
    level: ::std::os::raw::c_int,
) -> LinearCombination {
    schur_mult_fusion(sh1.data, sh2.data, rows, level).into()
}

pub(crate) fn _schur_skew(
    outer: &IntVector,
    inner: &IntVector,
    rows: ::std::os::raw::c_int,
    partsz: ::std::os::raw::c_int,
) -> LinearCombination {
    schur_skew(outer.data, inner.data, rows, partsz).into()
}

pub(crate) fn _schur_coprod(sh: &IntVector, all: bool) -> LinearCombination {
    schur_coprod(sh.data, sh.rows() as i32, sh.cols() as i32, -1, all).into()
}

pub(crate) fn _trans(w: &IntVector, vars: ::std::os::raw::c_int) -> LinearCombination {
    trans(w.data, vars).into()
}

pub(crate) fn _mult_schubert(
    ww1: &IntVector,
    ww2: &IntVector,
    rank: ::std::os::raw::c_int,
) -> LinearCombination {
    mult_schubert(ww1.data, ww2.data, rank).into()
}

pub(crate) fn _mult_schubert_str(ww1: &IntVector, ww2: &IntVector) -> LinearCombination {
    let ans = mult_schubert_str(ww1.data, ww2.data);
    if ans == std::ptr::null_mut() {
        panic!("Memory Error")
    }
    ans.into()
}

pub(crate) fn _fusion_reduce_lc(lc: &mut LinearCombination, level: ::std::os::raw::c_int) {
    fusion_reduce_lc(lc.data, level);
}

/// check that sh represents a partition, i.e., is weakly-decreasing and nonnegative
pub fn is_partition(sh: &[i32]) -> bool {
    if sh.len() == 0 {
        return true;
    }
    if sh[sh.len() - 1] < 0 {
        return false;
    }
    for i in 1..sh.len() {
        if sh[i - 1] < sh[i] {
            return false;
        }
    }
    true
}

/// check that p is a permutation of {1, 2, ..., n}
pub fn is_permutation(p: &[i32]) -> bool {
    let n = p.len() as i32;
    let mut flag = vec![false; n as usize];
    for &i in p {
        if i <= 0 || n < i || flag[(i - 1) as usize] {
            return false;
        }
        flag[(i - 1) as usize] = true;
    }
    true
}

pub fn are_compatible_strs(s1: &[i32], s2: &[i32]) -> bool {
    if s1.len() != s2.len() {
        return false;
    }
    if s1.len() == 0 {
        return true;
    }
    let n = *s1.iter().max().unwrap();
    if n != *s2.iter().max().unwrap() {
        return false;
    }
    let mut cnt = vec![0; (n + 1) as usize];
    for &a in s1 {
        if a < 0 {
            return false;
        }
        cnt[a as usize] += 1
    }
    for &a in s2 {
        if a < 0 {
            return false;
        }
        cnt[a as usize] -= 1
    }
    cnt.iter().all(|&c| c == 0)
}

/// outer, inner_1, inner_2 must be partitions
pub fn lrcoef(outer: &[i32], inner_1: &[i32], inner_2: &[i32]) -> i64 {
    let outer = IntVector::new(outer);
    let inner_1 = IntVector::new(inner_1);
    let inner_2 = IntVector::new(inner_2);
    debug_assert!(outer.is_partition());
    debug_assert!(inner_1.is_partition());
    debug_assert!(inner_2.is_partition());
    _schur_lrcoef(&outer, &inner_1, &inner_2)
}

/// sh1, sh2 must be partitions
pub fn mult(
    sh1: &[i32],
    sh2: &[i32],
    rows: Option<i32>,
    cols: Option<i32>,
) -> Vec<(Vec<i32>, i32)> {
    let sh1 = IntVector::new(sh1);
    let sh2 = IntVector::new(sh2);
    debug_assert!(sh1.is_partition());
    debug_assert!(sh2.is_partition());
    _schur_mult(&sh1, &sh2, rows.unwrap_or(-1), cols.unwrap_or(-1), -1)
        .map(|(k, v)| (k.to_partition(), v))
        .collect()
}

/// sh1, sh2 must be partitions
pub fn mult_fusion(sh1: &[i32], sh2: &[i32], rows: i32, level: i32) -> Vec<(Vec<i32>, i32)> {
    let sh1 = IntVector::new(sh1);
    let sh2 = IntVector::new(sh2);
    debug_assert!(sh1.is_partition());
    debug_assert!(sh2.is_partition());
    _schur_mult_fusion(&sh1, &sh2, rows, level)
        .map(|(k, v)| (k.to_partition(), v))
        .collect()
}

/// sh1, sh2 must be partitions
pub fn mult_quantum(sh1: &[i32], sh2: &[i32], rows: i32, cols: i32) -> Vec<((i32, Vec<i32>), i32)> {
    let sh1 = IntVector::new(sh1);
    let sh2 = IntVector::new(sh2);
    debug_assert!(sh1.is_partition());
    debug_assert!(sh2.is_partition());
    _schur_mult_fusion(&sh1, &sh2, rows, cols)
        .map(|(k, v)| (k.to_quantum(cols), v))
        .collect()
}

/// outer, inner must be partitions
pub fn skew(outer: &[i32], inner: &[i32], rows: Option<i32>) -> Vec<(Vec<i32>, i32)> {
    let outer = IntVector::new(outer);
    let inner = IntVector::new(inner);
    debug_assert!(outer.is_partition());
    debug_assert!(inner.is_partition());
    _schur_skew(&outer, &inner, rows.unwrap_or(-1), -1)
        .map(|(k, v)| (k.to_partition(), v))
        .collect()
}

/// outer, inner must be partitions
pub fn skew_tab(outer: &[i32], inner: &[i32], rows: Option<i32>) -> Vec<Vec<i32>> {
    let outer = IntVector::new(outer);
    let inner = IntVector::new(inner);
    debug_assert!(outer.is_partition());
    debug_assert!(inner.is_partition());
    LRTableauIterator::new(outer.data, inner.data, rows.unwrap_or(-1), -1, -1).collect()
}

/// sh must be a partition
pub fn coprod(sh: &[i32], all: Option<bool>) -> Vec<((Vec<i32>, Vec<i32>), i32)> {
    let sh = IntVector::new(sh);
    debug_assert!(sh.is_partition());
    _schur_coprod(&sh, all.unwrap_or(false))
        .map(|(k, v)| (k.to_pair(sh.cols()), v))
        .collect()
}

pub fn schubert_poly(perm: &[i32]) -> Vec<(Vec<i32>, i32)> {
    _trans(&IntVector::new(perm), 0)
        .map(|(k, v)| (k.to_vec(), v))
        .collect()
}

/// w1, w2 must be permutations and rank must be non-negative
pub fn schubmult(w1: &[i32], w2: &[i32], rank: Option<i32>) -> Vec<(Vec<i32>, i32)> {
    let w1 = IntVector::new(w1);
    let w2 = IntVector::new(w2);
    debug_assert!(w1.is_permutation());
    debug_assert!(w1.is_permutation());
    debug_assert!(rank.unwrap_or(0) >= 0);
    _mult_schubert(&w1, &w2, rank.unwrap_or(0))
        .map(|(k, v)| (k.to_vec(), v))
        .collect()
}

/// w1 and w2 must be compatible strings
pub fn schubmult_str(w1: &[i32], w2: &[i32]) -> Vec<(Vec<i32>, i32)> {
    let w1 = IntVector::new(w1);
    let w2 = IntVector::new(w2);
    debug_assert!(w1.is_compatible_str(&w2));
    _mult_schubert_str(&w1, &w2)
        .map(|(k, v)| (k.to_vec(), v))
        .collect()
}

pub fn all_parts(rows: i32, cols: i32) -> Vec<Vec<i32>> {
    let mut ans = Vec::new();
    let p = IntVector::default(rows as u32);
    let mut pitr = pitr_first_rs(p.data, rows, cols, std::ptr::null(), std::ptr::null(), 0, 0);
    while pitr_good(&pitr) {
        ans.push(p.to_vec());
        pitr_next(&mut pitr)
    }
    ans
}

pub fn all_perms(n: i32) -> Vec<Vec<i32>> {
    let ans = _all_perms(n);
    if ans == std::ptr::null_mut() {
        panic!("Memory Error")
    }
    let ans = VectorList { data: ans };
    (0..ans.len()).map(|i| ans.at(i).to_partition()).collect()
}

pub fn all_strings(dimvec: &[i32]) -> Vec<Vec<i32>> {
    let dimvec = IntVector::new(dimvec);
    debug_assert!(dimvec.is_dimvec());
    let ans = _all_strings(dimvec.data);
    if ans == std::ptr::null_mut() {
        panic!("Memory Error")
    }
    let ans = VectorList { data: ans };
    (0..ans.len()).map(|i| ans.at(i).to_vec()).collect()
}
