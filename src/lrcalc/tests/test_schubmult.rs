#![cfg(test)]

use anyhow::{Context, Result};

use super::super::{
    func::{_mult_poly_schubert, _mult_schubert, _trans, all_perms},
    ivector::IntVector,
};

pub fn test_mult_schubert(w1: &[i32], w2: &[i32]) -> Result<()> {
    let mut w1 = IntVector::new(w1);
    let mut w2 = IntVector::new(w2);
    let poly = _trans(&w1, 0);
    let prd12 = _mult_poly_schubert(poly, &mut w2, 0);
    let poly = _trans(&w2, 0);
    let prd21 = _mult_poly_schubert(poly, &mut w1, 0);
    prd12
        .diff(&prd21, |_, _| true)
        .expect_equals()
        .context("prd12 != prd21")?;
    drop(prd21);
    let maxrank = prd12
        .map
        .iter()
        .map(|(sh, _)| sh.ptr.perm_group())
        .max()
        .unwrap_or(0);
    for r in 0..=maxrank {
        let prd_sm = _mult_schubert(&mut w1, &mut w2, r);
        prd_sm
            .diff(&prd12, |sh, _| r == 0 || sh.perm_group() <= r)
            .expect_equals()
            .with_context(|| format!("prd_sm != prd12 at r={}", r))?;
    }
    Ok(())
}

pub fn run_test_schubmult(n: i32) -> Result<()> {
    let perms = all_perms(n);
    for w1 in perms.iter() {
        for w2 in perms.iter() {
            test_mult_schubert(w1, w2).with_context(|| {
                format!(
                    "Tests for schubmult failed at w1={:?}, w2={:?}",
                    w1.to_vec(),
                    w2.to_vec(),
                )
            })?;
        }
    }
    Ok(())
}
