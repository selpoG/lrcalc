#![cfg(test)]

use anyhow::{ensure, Context, Result};

use super::super::{
    func::{_schur_lrcoef, _schur_skew},
    ivector::IntVector,
    part::PartIter,
};

pub fn test_schur_lrskew(outer: &IntVector, inner: &IntVector, rows: i32, cols: i32) -> Result<()> {
    let lc = _schur_skew(outer, inner, -1, rows);

    let mut pitr = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(sh) = pitr.next() {
        let coef = _schur_lrcoef(outer, inner, &sh);
        ensure!(coef >= 0, "memory error: sh={:?}", sh.to_vec());
        let expected = if let Some(d) = lc.find(&sh[..]) {
            d.value as i64
        } else {
            0
        };
        ensure!(
            coef == expected,
            "coef mismatch: outer={:?}, coef={}, expected={}",
            sh.to_vec(),
            coef,
            expected
        );
    }
    for r in 0..rows {
        lc.diff(&_schur_skew(outer, inner, r, rows), |sh, _| {
            sh.rows() <= r as usize
        })
        .expect_equals()
        .with_context(|| format!("lc != lc_r at r={}", r))?;
    }
    Ok(())
}

pub fn run_test_lrskew(rows: i32, cols: i32) -> Result<()> {
    let mut pitr1 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(p1) = pitr1.next() {
        let mut pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
        while let Some(p2) = pitr2.next() {
            test_schur_lrskew(&p1, &p2, rows, cols)
                .with_context(|| format!("p1={:?}, p2={:?}", p1.to_vec(), p2.to_vec()))?;
        }
    }
    Ok(())
}
