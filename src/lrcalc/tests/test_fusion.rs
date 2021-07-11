#![cfg(test)]

use anyhow::{Context, Result};

use super::super::{
    ivector::IntVector,
    part::PartIter,
    schur::{fusion_reduce_lc, schur_mult, schur_mult_fusion},
};

pub fn test_mult_fusion(sh1: &IntVector, sh2: &IntVector, rows: i32, level: i32) -> Result<()> {
    let prd_f = schur_mult_fusion(&sh1, &sh2, rows, level);
    let mut prd_s = schur_mult(&sh1, &sh2, rows, -1, rows);
    fusion_reduce_lc(&mut prd_s, level);
    prd_f
        .diff(&prd_s, |_, _| true)
        .expect_equals()
        .context("prd_f != prd_s")?;
    Ok(())
}

pub fn run_test_mult_fusion(rows: i32, cols: i32) -> Result<()> {
    let pitr1 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    for p1 in pitr1 {
        let pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
        for p2 in pitr2 {
            for level in 0..=cols {
                test_mult_fusion(&p1, &p2, rows, level).with_context(|| {
                    format!(
                        "p1={:?}, p2={:?}, level={:?}",
                        p1.to_vec(),
                        p2.to_vec(),
                        level
                    )
                })?;
            }
        }
    }
    Ok(())
}
