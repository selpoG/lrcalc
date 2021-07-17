#![cfg(test)]

use anyhow::{ensure, Context, Result};

use super::super::{
    ivector::IntVector,
    part::PartIter,
    schur::{schur_lrcoef, schur_skew},
};

pub fn test_schur_lrskew(outer: &IntVector, inner: &IntVector, rows: i32, cols: i32) -> Result<()> {
    let lc = schur_skew(outer, inner, -1, rows);

    let pitr = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    for sh in pitr {
        let coef = schur_lrcoef(outer, inner, &sh);
        ensure!(coef >= 0, "memory error: sh={:?}", sh.to_vec());
        let expected = if let Some(d) = lc.find(&sh[..]) {
            d as i64
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
        lc.diff(&schur_skew(outer, inner, r, rows), |sh, _| {
            sh.rows() <= r as usize
        })
        .expect_equals()
        .with_context(|| format!("lc != lc_r at r={}", r))?;
    }
    Ok(())
}

pub fn run_test_lrskew(rows: i32, cols: i32) -> Result<()> {
    let pitr1 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    for p1 in pitr1 {
        let pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
        for p2 in pitr2 {
            test_schur_lrskew(&p1, &p2, rows, cols)
                .with_context(|| format!("p1={:?}, p2={:?}", p1.to_vec(), p2.to_vec()))?;
        }
    }
    Ok(())
}
