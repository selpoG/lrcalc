#![cfg(test)]

use anyhow::{Context, Result};

use super::super::{
    ivector::IntVector, ivlincomb::LinearCombination, part::PartIter, schublib::mult_schubert_str,
    schur::schur_mult,
};

fn part_to_string(p: &IntVector, rows: i32, cols: i32) -> IntVector {
    let mut s = vec![1; (rows + cols) as usize];
    let v = &p[..];
    for i in 0..rows {
        let idx = (rows - 1 - i) as usize;
        s[(i + if idx >= v.len() { 0 } else { v[idx] }) as usize] = 0;
    }
    IntVector::new(&s[..])
}

fn string_to_part(s: &IntVector, rows: i32) -> Vec<i32> {
    let mut p = vec![0; rows as usize];
    let mut i = 0;
    let v = &s[..];
    for (j, &x) in v.iter().enumerate() {
        if x == 0 {
            p[(rows - 1 - i) as usize] = j as i32 - i;
            i += 1
        }
    }
    p
}

pub fn test_schur_mult(p1: &IntVector, p2: &IntVector) -> Result<()> {
    let rows = (p1.rows() + p2.rows()) as i32;
    let cols = (p1.cols() + p2.cols()) as i32;
    let prd = LinearCombination::new(
        mult_schubert_str(
            &part_to_string(p1, rows, cols),
            &part_to_string(p2, rows, cols),
        )
        .unwrap()
        .map(&|k, v| (string_to_part(&k, rows), v)),
    );
    schur_mult(&p1, &p2, -1, -1, -1)
        .diff(&prd, |_, _| true)
        .expect_equals()
        .context("prd_sm != prd")?;
    for r in -1..=rows {
        for c in -1..=cols {
            schur_mult(&p1, &p2, r, c, rows)
                .diff(&prd, |key, _| {
                    (r == -1 || key.rows() <= r as usize) && (c == -1 || key.cols() <= c as usize)
                })
                .expect_equals()
                .with_context(|| format!("prd_sm != prd at r={}, c={}", r, c))?;
        }
    }
    Ok(())
}

pub fn run_test_lrmult(rows: i32, cols: i32) -> Result<()> {
    let pitr1 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    for p1 in pitr1 {
        let pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
        for p2 in pitr2 {
            test_schur_mult(&p1, &p2)
                .with_context(|| format!("p1={:?}, p2={:?}", p1.to_vec(), p2.to_vec()))?;
        }
    }
    Ok(())
}
