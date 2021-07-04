#![cfg(test)]

mod test_fusion;
mod test_lrcoef;
mod test_lrmult;
mod test_lrskew;
mod test_partiter;
mod test_schubmult;

use super::func::*;
use anyhow::{Context, Result};

fn cmp_unordered<T: Eq + Ord + std::fmt::Debug + Clone>(a: &[T], b: &[T]) {
    let mut a = a.to_vec();
    let mut b = b.to_vec();
    a.sort();
    b.sort();
    assert_eq!(a, b);
}

#[test]
fn test_lrcalc() {
    assert_eq!(lrcoef(&[3, 2, 1], &[2, 1], &[2, 1]), 2);
    cmp_unordered(
        &skew(&[3, 2, 1], &[2, 1], None),
        &[(vec![2, 1], 2), (vec![1, 1, 1], 1), (vec![3], 1)],
    );
    cmp_unordered(
        &skew(&[3, 2, 1], &[2, 1], Some(2)),
        &[(vec![2, 1], 2), (vec![3], 1)],
    );
    cmp_unordered(
        &mult(&[2, 1], &[2, 1], None, None),
        &[
            (vec![3, 2, 1], 2),
            (vec![4, 2], 1),
            (vec![3, 1, 1, 1], 1),
            (vec![3, 3], 1),
            (vec![2, 2, 1, 1], 1),
            (vec![2, 2, 2], 1),
            (vec![4, 1, 1], 1),
        ],
    );
    cmp_unordered(
        &mult_fusion(&[3, 2, 1], &[3, 2, 1], 3, 2),
        &[(vec![4, 4, 4], 1), (vec![5, 4, 3], 1)],
    );
    cmp_unordered(
        &mult_quantum(&[3, 2, 1], &[3, 2, 1], 3, 2),
        &[((2, vec![2]), 1), ((2, vec![1, 1]), 1)],
    );
    cmp_unordered(
        &coprod(&[3, 2, 1], None),
        &[
            (vec![2, 1, 1], vec![1, 1], 1),
            (vec![3, 2, 1], vec![], 1),
            (vec![3, 1, 1], vec![1], 1),
            (vec![2, 2], vec![1, 1], 1),
            (vec![2, 1], vec![2, 1], 2),
            (vec![3, 2], vec![1], 1),
            (vec![3, 1], vec![2], 1),
            (vec![1, 1, 1], vec![2, 1], 1),
            (vec![2, 2, 1], vec![1], 1),
            (vec![2, 1, 1], vec![2], 1),
            (vec![2, 2], vec![2], 1),
            (vec![2, 1], vec![3], 1),
            (vec![3, 1], vec![1, 1], 1),
        ],
    );
}

#[test]
fn test_schubmult() {
    cmp_unordered(
        &schubmult(&[1, 3, 2], &[1, 3, 2], None),
        &[(vec![2, 3, 1], 1), (vec![1, 4, 2, 3], 1)],
    );
    cmp_unordered(
        &schubmult(&[1, 3, 4, 2], &[2, 1, 4, 5, 3], None),
        &[
            (vec![2, 3, 5, 4, 1], 1),
            (vec![4, 1, 5, 2, 3], 1),
            (vec![2, 4, 5, 1, 3], 1),
            (vec![3, 2, 4, 5, 1], 1),
            (vec![3, 1, 5, 4, 2], 1),
        ],
    );
    cmp_unordered(
        &schubmult_str(&[0, 1, 2, 0, 1, 2], &[0, 1, 2, 0, 1, 2]),
        &[
            (vec![2, 1, 0, 0, 1, 2], 1),
            (vec![1, 0, 2, 2, 0, 1], 1),
            (vec![0, 2, 1, 1, 2, 0], 1),
            (vec![2, 0, 1, 1, 0, 2], 1),
            (vec![1, 2, 0, 0, 2, 1], 1),
            (vec![0, 1, 2, 2, 1, 0], 1),
        ],
    );
    cmp_unordered(
        &schubmult_str(&[0, 2, 0, 2, 1, 2], &[0, 1, 2, 0, 2, 2]),
        &[
            (vec![0, 2, 1, 2, 2, 0], 1),
            (vec![2, 0, 1, 2, 0, 2], 1),
            (vec![2, 0, 2, 0, 1, 2], 1),
            (vec![0, 2, 2, 1, 0, 2], 1),
        ],
    );
}

#[test]
fn test_allperms() {
    assert_eq!(all_perms(0), vec![vec![]]);
    assert_eq!(all_perms(1), vec![vec![1]]);
    assert_eq!(
        all_perms(3),
        vec![
            vec![1, 2, 3],
            vec![1, 3, 2],
            vec![2, 1, 3],
            vec![2, 3, 1],
            vec![3, 1, 2],
            vec![3, 2, 1]
        ]
    );
}

#[test]
fn run_tests() -> Result<()> {
    for &(r, c) in [(0, 0), (0, 3), (2, 0), (1, 1), (4, 4)].iter() {
        test_partiter::run_test_partiter(r, c)
            .with_context(|| format!("run_test_partiter({}, {})", r, c))?;
    }

    for &n in [1, 2, 4].iter() {
        test_schubmult::run_test_schubmult(n)
            .with_context(|| format!("run_test_schubmult({})", n))?;
    }

    for &(r, c) in [(2, 4), (3, 3), (4, 2)].iter() {
        test_lrmult::run_test_lrmult(r, c)
            .with_context(|| format!("run_test_lrmult({}, {})", r, c))?;
    }

    test_lrcoef::run_test_lrcoef(4, 4).context("run_test_lrcoef(4, 4)")?;

    test_lrskew::run_test_lrskew(4, 4).context("run_test_lrskew(4, 4)")?;

    for &(r, c) in [(0, 5), (1, 5), (2, 5), (3, 5), (4, 5)].iter() {
        test_fusion::run_test_mult_fusion(r, c)
            .with_context(|| format!("run_test_mult_fusion({}, {})", r, c))?;
    }
    Ok(())
}
