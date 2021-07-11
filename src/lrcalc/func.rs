use super::{
    ivector::IntVector,
    lriter::LRTableauIterator,
    part::PartitionIterator,
    perm::{all_perms as _all_perms, all_strings as _all_strings},
    schublib::{mult_schubert, mult_schubert_str, trans},
    schur::{schur_coprod, schur_lrcoef, schur_mult, schur_mult_fusion, schur_skew},
};

/// check that sh represents a partition, i.e., is weakly-decreasing and nonnegative
pub fn is_partition(sh: &[i32]) -> bool {
    if sh.is_empty() {
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

/// outer, inner_1, inner_2 must be partitions
pub fn lrcoef(outer: &[i32], inner_1: &[i32], inner_2: &[i32]) -> i64 {
    let outer = IntVector::new(outer);
    let inner_1 = IntVector::new(inner_1);
    let inner_2 = IntVector::new(inner_2);
    debug_assert!(outer.is_partition());
    debug_assert!(inner_1.is_partition());
    debug_assert!(inner_2.is_partition());
    schur_lrcoef(&outer, &inner_1, &inner_2)
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
    schur_mult(&sh1, &sh2, rows.unwrap_or(-1), cols.unwrap_or(-1), -1)
        .map(&|k, v| (k.to_partition(), v))
}

/// sh1, sh2 must be partitions
pub fn mult_fusion(sh1: &[i32], sh2: &[i32], rows: i32, level: i32) -> Vec<(Vec<i32>, i32)> {
    let sh1 = IntVector::new(sh1);
    let sh2 = IntVector::new(sh2);
    debug_assert!(sh1.is_partition());
    debug_assert!(sh2.is_partition());
    schur_mult_fusion(&sh1, &sh2, rows, level).map(&|k, v| (k.to_partition(), v))
}

/// sh1, sh2 must be partitions
pub fn mult_quantum(sh1: &[i32], sh2: &[i32], rows: i32, cols: i32) -> Vec<((i32, Vec<i32>), i32)> {
    let sh1 = IntVector::new(sh1);
    let sh2 = IntVector::new(sh2);
    debug_assert!(sh1.is_partition());
    debug_assert!(sh2.is_partition());
    schur_mult_fusion(&sh1, &sh2, rows, cols).map(&|k, v| (k.to_quantum(cols), v))
}

/// outer, inner must be partitions
pub fn skew(outer: &[i32], inner: &[i32], rows: Option<i32>) -> Vec<(Vec<i32>, i32)> {
    let outer = IntVector::new(outer);
    let inner = IntVector::new(inner);
    debug_assert!(outer.is_partition());
    debug_assert!(inner.is_partition());
    schur_skew(&outer, &inner, rows.unwrap_or(-1), -1).map(&|k, v| (k.to_partition(), v))
}

/// outer, inner must be partitions
pub fn skew_tab(outer: &[i32], inner: &[i32], rows: Option<i32>) -> Vec<Vec<i32>> {
    let outer = IntVector::new(outer);
    let inner = IntVector::new(inner);
    debug_assert!(outer.is_partition());
    debug_assert!(inner.is_partition());
    LRTableauIterator::new(&outer, Some(&inner), rows.unwrap_or(-1), -1, -1).collect()
}

/// sh must be a partition
pub fn coprod(sh: &[i32], all: Option<bool>) -> Vec<(Vec<i32>, Vec<i32>, i32)> {
    let sh = IntVector::new(sh);
    debug_assert!(sh.is_partition());
    schur_coprod(
        &sh,
        sh.rows() as i32,
        sh.cols() as i32,
        -1,
        all.unwrap_or(false),
    )
    .map(&|k, v| {
        let (a, b) = k.to_pair(sh.cols());
        (a, b, v)
    })
}

pub fn schubert_poly(perm: &[i32]) -> Vec<(Vec<i32>, i32)> {
    trans(&IntVector::new(perm)[..], 0).map(&|k, v| (k.to_vec(), v))
}

/// w1, w2 must be permutations and rank must be non-negative
pub fn schubmult(w1: &[i32], w2: &[i32], rank: Option<i32>) -> Vec<(Vec<i32>, i32)> {
    let mut w1 = IntVector::new(w1);
    let mut w2 = IntVector::new(w2);
    debug_assert!(w1.is_permutation());
    debug_assert!(w1.is_permutation());
    debug_assert!(rank.unwrap_or(0) >= 0);
    mult_schubert(&mut w1, &mut w2, rank.unwrap_or(0)).map(&|k, v| (k.to_vec(), v))
}

/// w1 and w2 must be compatible strings
pub fn schubmult_str(w1: &[i32], w2: &[i32]) -> Vec<(Vec<i32>, i32)> {
    let w1 = IntVector::new(w1);
    let w2 = IntVector::new(w2);
    debug_assert!(w1.is_compatible_str(&w2));
    mult_schubert_str(&w1, &w2)
        .unwrap()
        .map(&|k, v| (k.to_vec(), v))
}

pub fn all_parts(rows: i32, cols: i32) -> Vec<Vec<i32>> {
    let mut ans = Vec::new();
    let p = IntVector::default(rows as u32);
    let pitr = PartitionIterator::new(p, rows, cols, None, None, 0, 0);
    let mut pitr = match pitr {
        None => return ans,
        Some(x) => x,
    };
    while pitr.is_good() {
        ans.push(pitr.part[..].to_vec());
        pitr.next();
    }
    ans
}

pub fn all_perms(n: i32) -> Vec<Vec<i32>> {
    _all_perms(n)
        .unwrap()
        .iter()
        .map(|v| v.to_partition())
        .collect()
}

pub fn all_strings(dimvec: &[i32]) -> Vec<Vec<i32>> {
    let dimvec = IntVector::new(dimvec);
    debug_assert!(dimvec.is_dimvec());
    _all_strings(&dimvec[..])
        .unwrap()
        .iter()
        .map(|v| v.to_vec())
        .collect()
}
