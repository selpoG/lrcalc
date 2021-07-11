use super::super::lriter::{lrit_expand, lrit_good, lrit_new, lrit_next, LRTableauIterator};
use super::ivector::{iv_hash, IntVector};
use super::ivlincomb::{
    ivlc_add_element, ivlc_new, ivlc_new_default, LinearCombination, LC_COPY_KEY, LC_FREE_KEY,
    LC_FREE_ZERO,
};
use super::lrcoef::lrcoef_count;
use super::optim::{optim_coef, optim_fusion, optim_mult, optim_skew};
use super::part::{part_entry, part_valid};

pub fn schur_mult(
    sh1: &IntVector,
    sh2: &IntVector,
    rows: i32,
    cols: i32,
    partsz: i32,
) -> LinearCombination {
    let ss = optim_mult(sh1, Some(sh2), rows, cols);
    if ss.sign != 0 {
        lrit_expand(
            ss.outer.as_ref().unwrap(),
            None,
            ss.cont.as_ref(),
            rows,
            cols,
            partsz,
        )
    } else {
        ivlc_new(2)
    }
}

fn fusion_reduce(la: &mut IntVector, level: i32, tmp: &mut [i32]) -> i32 {
    debug_assert!(la.length == tmp.len() as u32);
    let rows = la.length as i32;
    let n = rows + level;

    let mut q = 0;
    for i in 0..rows {
        let a = la[i as usize] + rows - i - 1;
        let b = if a >= 0 { a / n } else { -((n - 1 - a) / n) };
        q += b;
        tmp[i as usize] = a - b * n - rows + 1;
    }

    /* bubble sort */
    let mut sign = if (rows & 1) != 0 { 0 } else { q };
    for i in 0..(rows - 1) {
        let mut k = i;
        let mut a = tmp[k as usize];
        for j in (i + 1)..rows {
            if a < tmp[j as usize] {
                k = j;
                a = tmp[k as usize];
            }
        }
        if k != i {
            tmp[k as usize] = tmp[i as usize];
            tmp[i as usize] = a;
            sign += 1;
        }
    }

    for i in 0..rows {
        if i > 0 && tmp[(i - 1) as usize] == tmp[i as usize] {
            return 0;
        }
        let k = i + q;
        let a = tmp[i as usize] + k + (k / rows) * level;
        la[((k + rows) % rows) as usize] = a;
    }

    if (sign & 1) != 0 {
        -1
    } else {
        1
    }
}

pub fn fusion_reduce_lc(lc: &mut LinearCombination, level: i32) {
    let card = lc.map.len();
    let mut parts = Vec::with_capacity(card);
    let mut coefs = Vec::with_capacity(card);

    for kv in lc.map.drain() {
        parts.push(kv.0.ptr);
        coefs.push(kv.1);
    }

    if parts.is_empty() {
        return;
    }
    let mut tmp = {
        let sh = &parts[0];
        vec![0i32; sh.length as usize]
    };

    /* Reduce and reinsert terms. */
    while !parts.is_empty() {
        let mut sh = parts.pop().unwrap();
        let c = coefs.pop().unwrap();
        let sign = fusion_reduce(&mut sh, level, &mut tmp[..]);
        let hash = iv_hash(&sh[..]);
        ivlc_add_element(lc, sign * c, sh, hash, LC_FREE_KEY | LC_FREE_ZERO);
    }
}

pub fn schur_mult_fusion(
    sh1: &IntVector,
    sh2: &IntVector,
    rows: i32,
    level: i32,
) -> LinearCombination {
    let mut sh1 = sh1;
    let mut sh2 = sh2;
    debug_assert!(part_valid(&sh1[..]) && part_valid(&sh2[..]));
    if part_entry(&sh1[..], rows) != 0 || part_entry(&sh2[..], rows) != 0 {
        return ivlc_new(2);
    }

    let mut sign = 1;
    let mut tmp: Option<IntVector> = None;
    let mut nsh1: Option<IntVector>;
    let mut nsh2: Option<IntVector>;
    if part_entry(&sh1[..], 0) - part_entry(&sh1[..], rows - 1) > level {
        tmp = Some(vec![0; rows as usize].into());
        nsh1 = Some(vec![0; rows as usize].into());
        for i in 0..rows {
            (nsh1.as_mut().unwrap())[i as usize] = part_entry(&sh1[..], i);
        }
        sign = fusion_reduce(
            &mut nsh1.as_mut().unwrap(),
            level,
            &mut (tmp.as_mut().unwrap())[..],
        );
        sh1 = nsh1.as_ref().unwrap();
    }
    if sign == 0 {
        return ivlc_new(2);
    }
    if part_entry(&sh2[..], 0) - part_entry(&sh2[..], rows - 1) > level {
        if tmp.is_none() {
            tmp = Some(vec![0; rows as usize].into());
        }
        nsh2 = Some(vec![0; rows as usize].into());
        for i in 0..rows {
            (nsh2.as_mut().unwrap())[i as usize] = part_entry(&sh2[..], i);
        }
        sign *= fusion_reduce(
            &mut nsh2.as_mut().unwrap(),
            level,
            &mut (tmp.as_mut().unwrap())[..],
        );
        sh2 = nsh2.as_ref().unwrap();
    }
    if sign == 0 {
        return ivlc_new(2);
    }

    let ss = optim_fusion(sh1, sh2, rows, level);
    let mut lc = if ss.sign != 0 {
        lrit_expand(
            ss.outer.as_ref().unwrap(),
            None,
            ss.cont.as_ref(),
            rows,
            -1,
            rows,
        )
    } else {
        ivlc_new(2)
    };

    fusion_reduce_lc(&mut lc, level);

    if sign < 0 {
        for kv in lc.map.iter_mut() {
            *kv.1 = -*kv.1;
        }
    }

    lc
}

pub fn schur_skew(
    outer: &IntVector,
    inner: &IntVector,
    rows: i32,
    partsz: i32,
) -> LinearCombination {
    let ss = optim_skew(outer, Some(inner), None, rows);
    if ss.sign != 0 {
        lrit_expand(
            ss.outer.as_ref().unwrap(),
            ss.inner.as_ref(),
            ss.cont.as_ref(),
            rows,
            -1,
            partsz,
        )
    } else {
        ivlc_new(2)
    }
}

fn _schur_coprod_isredundant(cont: &IntVector, rows: i32, cols: i32) -> bool {
    let mut sz1 = -rows * cols;
    for i in 0..rows {
        sz1 += cont[i as usize];
    }
    let mut sz2 = 0;
    for i in rows..cont.length as i32 {
        sz2 += cont[i as usize];
    }
    if sz1 != sz2 {
        return sz1 < sz2;
    }
    for i in 0..rows {
        let df = cont[i as usize] - cols - part_entry(&cont[..], rows + i);
        if df != 0 {
            return df > 0;
        }
    }
    false
}

fn _schur_coprod_count(lrit: &mut LRTableauIterator, rows: i32, cols: i32) -> LinearCombination {
    let mut lc = ivlc_new_default();
    while lrit_good(lrit) {
        if _schur_coprod_isredundant(&lrit.cont, rows, cols) {
            lrit_next(lrit);
            continue;
        }
        ivlc_add_element(
            &mut lc,
            1,
            lrit.cont.clone(),
            iv_hash(&lrit.cont[..]),
            LC_COPY_KEY,
        );
        lrit_next(lrit);
    }
    lc
}

fn _schur_coprod_expand(
    outer: &IntVector,
    content: Option<&IntVector>,
    rows: i32,
    cols: i32,
    partsz: i32,
) -> LinearCombination {
    let mut lrit = lrit_new(outer, None, content, -1, -1, partsz);
    _schur_coprod_count(&mut lrit, rows, cols)
}

pub fn schur_coprod(
    sh: &IntVector,
    rows: i32,
    cols: i32,
    partsz: i32,
    all: bool,
) -> LinearCombination {
    let b = &vec![cols; rows as usize].into();

    if all {
        return schur_mult(sh, b, -1, -1, partsz);
    }

    let ss = optim_mult(sh, Some(b), -1, -1);

    _schur_coprod_expand(
        ss.outer.as_ref().unwrap(),
        ss.cont.as_ref(),
        rows,
        cols,
        partsz,
    )
}

pub fn schur_lrcoef(outer: &IntVector, inner1: &IntVector, inner2: &IntVector) -> i64 {
    let ss = optim_coef(outer, inner1, inner2);
    if ss.sign <= 1 {
        ss.sign as i64
    } else {
        lrcoef_count(
            ss.outer.as_ref().unwrap(),
            ss.inner.as_ref().unwrap(),
            ss.cont.as_ref().unwrap(),
        )
    }
}
