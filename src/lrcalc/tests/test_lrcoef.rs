#![cfg(test)]

use anyhow::{ensure, Context, Result};

use super::super::{
    func::{_schur_lrcoef, _schur_mult},
    ivector::IntVector,
    part::PartIter,
};

pub fn test_schur_lrcoef(p1: &IntVector, p2: &IntVector, rows: i32, cols: i32) -> Result<()> {
    let prd = _schur_mult(p1, p2, rows, cols, rows);

    let mut pitr = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(outer) = pitr.next() {
        let coef = _schur_lrcoef(&outer, p1, p2);
        ensure!(coef >= 0, "memory error: outer={:?}", outer.to_vec());
        let expected = if let Some(d) = prd.find(&outer[..]) {
            d as i64
        } else {
            0
        };
        ensure!(
            coef == expected,
            "coef mismatch: outer={:?}, coef={}, expected={}",
            outer.to_vec(),
            coef,
            expected
        );
    }
    Ok(())
}

pub fn run_test_lrcoef(rows: i32, cols: i32) -> Result<()> {
    let mut pitr1 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(p1) = pitr1.next() {
        let mut pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
        while let Some(p2) = pitr2.next() {
            test_schur_lrcoef(&p1, &p2, rows, cols)
                .with_context(|| format!("p1={:?}, p2={:?}", p1.to_vec(), p2.to_vec()))?;
        }
    }
    Ok(())
}
