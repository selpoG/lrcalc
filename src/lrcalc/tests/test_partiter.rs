#![cfg(test)]

use anyhow::{ensure, Context, Result};

use super::super::{
    helper::part::{PITR_USE_INNER, PITR_USE_OUTER, PITR_USE_SIZE},
    ivector::IntVector,
    part::PartIter,
};

fn test_part_iter_box(rows: i32, cols: i32) -> Result<()> {
    let mut np = 1;
    for i in 1..=rows {
        np = np * (cols + i) / i;
    }
    let mut np1 = 0;
    let mut pitr = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(p) = pitr.next() {
        ensure!(
            p.is_partition(),
            "not a partition from new_box: p={:?}",
            p.to_vec()
        );
        np1 += 1;
    }
    ensure!(
        np == np1,
        "num of partitions given by new_box is wrong: counted={}, expected={}",
        np1,
        np
    );
    np1 = 0;
    for size in 0..(rows * cols + 2) {
        let mut pitr = PartIter::new_box_sz(IntVector::default(rows as u32), rows, cols, size);
        while let Some(p) = pitr.next() {
            ensure!(
                p.is_partition(),
                "not a partition from new_box: size={}, p={:?}",
                size,
                p.to_vec()
            );
            ensure!(
                p.size() == size,
                "wrong size from new_box: size={}, p={:?}",
                size,
                p.to_vec()
            );
            np1 += 1;
        }
    }
    ensure!(
        np == np1,
        "num of partitions given by new_box_sz is wrong: counted={}, expected={}",
        np1,
        np
    );
    Ok(())
}

fn test_part_iter_between(
    rows: i32,
    cols: i32,
    outer: Option<&IntVector>,
    inner: Option<&IntVector>,
) -> Result<()> {
    let check_outer = |p: &IntVector| match outer {
        Some(outer) => p.leq_as_part(outer),
        None => true,
    };
    let check_inner = |p: &IntVector| match inner {
        Some(inner) => inner.leq_as_part(&p),
        None => true,
    };
    let pitr_flag = if outer.is_some() { PITR_USE_OUTER } else { 0 }
        | if inner.is_some() { PITR_USE_INNER } else { 0 };
    let size_bound = match outer {
        Some(outer) => outer.size() + 2,
        None => rows * cols + 2,
    };
    let mut count = vec![0; size_bound as usize];
    let mut pitr = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(p) = pitr.next() {
        if check_inner(&p) && check_outer(&p) {
            count[p.size() as usize] += 1
        }
    }
    let mut np = 0;
    let mut pitr = PartIter::new(
        IntVector::default(rows as u32),
        rows,
        cols,
        outer,
        inner,
        0,
        pitr_flag,
    );
    while let Some(p) = pitr.next() {
        ensure!(p.is_partition(), "invalid partition: p={:?}", p.to_vec());
        ensure!(
            check_inner(&p) && check_outer(&p),
            "inner <= p={:?} <= outer does not hold",
            p.to_vec()
        );
        np += 1;
    }
    let np1 = count.iter().sum();
    ensure!(
        np == np1,
        "num of partitions did not match: counted={}, expected={}",
        np,
        np1
    );
    for size in 0..size_bound {
        np = 0;
        let mut pitr = PartIter::new(
            IntVector::default(rows as u32),
            rows,
            cols,
            outer,
            inner,
            size,
            pitr_flag | PITR_USE_SIZE,
        );
        while let Some(p) = pitr.next() {
            ensure!(
                p.is_partition(),
                "invalid partition: size={}, p={:?}",
                size,
                p.to_vec()
            );
            ensure!(
                check_inner(&p) && check_outer(&p),
                "size={}, inner <= p={:?} <= outer does not hold",
                size,
                p.to_vec()
            );
            ensure!(
                p.size() == size,
                "invalid size: size={}, p={:?}",
                size,
                p.to_vec()
            );
            np += 1;
        }
        ensure!(
            np == count[size as usize],
            "num of partitions did not match: size={}, sum={}, expected={}",
            size,
            np,
            count[size as usize]
        );
    }
    Ok(())
}

#[cfg(test)]
pub fn run_test_partiter(rows: i32, cols: i32) -> Result<()> {
    let rows0 = if rows > 0 { rows - 1 } else { 0 };
    let cols0 = if cols > 0 { cols - 1 } else { 0 };
    test_part_iter_box(rows, cols).context("partiter_box")?;
    let mut pitr2 = PartIter::new_box(IntVector::default(rows as u32), rows, cols);
    while let Some(p2) = pitr2.next() {
        let test_sub = |r: i32, c: i32| -> Result<()> {
            test_part_iter_between(r, c, Some(&p2), None)
                .with_context(|| format!("rows={}, cols={}, outer={:?}", r, c, p2.to_vec()))
                .context("partiter_sub")
        };
        test_sub(rows, cols)?;
        test_sub(rows0, cols)?;
        test_sub(rows, cols0)?;
        test_sub(rows, cols + 1)?;
        let test_super = |r: i32, c: i32| -> Result<()> {
            test_part_iter_between(r, c, None, Some(&p2))
                .with_context(|| format!("rows={}, cols={}, inner={:?}", r, c, p2.to_vec()))
                .context("partiter_super")
        };
        test_super(rows, cols)?;
        test_super(rows0, cols)?;
        test_super(rows, cols0)?;
        test_super(rows, cols + 2)?;
        let mut pitr1 = PartIter::new(
            IntVector::default(rows as u32),
            rows,
            cols,
            Some(&p2),
            None,
            0,
            PITR_USE_OUTER,
        );
        while let Some(p1) = pitr1.next() {
            let test_between = |r: i32, c: i32| -> Result<()> {
                test_part_iter_between(r, c, Some(&p2), Some(&p1))
                    .with_context(|| {
                        format!(
                            "rows={}, cols={}, outer={:?}, inner={:?}",
                            r,
                            c,
                            p2.to_vec(),
                            p1.to_vec()
                        )
                    })
                    .context("partiter_between")
            };
            test_between(rows, cols)?;
            test_between(rows0, cols)?;
            test_between(rows, cols0)?;
            test_between(rows, cols + 2)?;
        }
    }
    Ok(())
}
