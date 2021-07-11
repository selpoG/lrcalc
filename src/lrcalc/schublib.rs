use super::helper::ivector::{iv_hash, iv_new, IntVector};

use super::{
    ivlincomb::{
        _ivlc_insert, ivlc_add_element, ivlc_add_multiple, ivlc_new_default, LinearCombination,
        LC_FREE_ZERO,
    },
    perm::{
        bruhat_zero, perm2string, perm_group, perm_length, str2dimvec, str_iscompat, string2perm,
    },
};

pub fn trans(w: &[i32], vars: i32) -> LinearCombination {
    let mut res = ivlc_new_default();
    _trans(&mut w.to_vec()[..], vars, &mut res);
    res
}

fn _trans(w: &mut [i32], mut vars: i32, res: &mut LinearCombination) {
    res.clear();

    let n = perm_group(w);

    let mut r = n - 1;
    while r > 0 && w[(r - 1) as usize] < w[r as usize] {
        r -= 1;
    }
    if r <= 0 {
        let xx = iv_new(std::cmp::max(vars as u32, 1));
        let hash = iv_hash(&xx[..]);
        _ivlc_insert(res, xx, hash, 1);
        return;
    }
    vars = std::cmp::max(vars, r);

    let mut s = r + 1;
    while s < n && w[(r - 1) as usize] > w[s as usize] {
        s += 1;
    }

    let wr = w[(r - 1) as usize];
    let ws = w[(s - 1) as usize];
    w[(s - 1) as usize] = wr;
    w[(r - 1) as usize] = ws;

    let mut tmp = trans(w, vars);
    for kv in tmp.map.drain() {
        let mut vec = kv.0.ptr;
        vec[(r - 1) as usize] += 1;
        let hash = iv_hash(&vec[..]);
        _ivlc_insert(res, vec, hash, kv.1);
    }

    let mut last = 0;
    let vr = w[(r - 1) as usize];
    for i in (1..r).rev() {
        let vi = w[(i - 1) as usize];
        if last < vi && vi < vr {
            last = vi;
            w[(i - 1) as usize] = vr;
            w[(r - 1) as usize] = vi;
            _trans(w, vars, &mut tmp);
            ivlc_add_multiple(res, 1, &mut tmp, LC_FREE_ZERO);
            w[(i - 1) as usize] = vi;
        }
    }

    w[(s - 1) as usize] = ws;
    w[(r - 1) as usize] = wr;
}

fn _monk_add(i: u32, slc: &LinearCombination, rank: i32, res: &mut LinearCombination) {
    let mut add = |vec: Vec<i32>, c: i32| {
        let vec: IntVector = vec.into();
        let hash = iv_hash(&vec[..]);
        ivlc_add_element(res, c, vec, hash, LC_FREE_ZERO)
    };
    for kv in slc.map.iter() {
        let w = &(kv.0.ptr)[..];
        let c = *kv.1;
        let n = w.len() as u32;
        let wi = if i <= n {
            w[(i - 1) as usize]
        } else {
            i as i32
        };

        if i <= n + 1 {
            let mut last = 0;
            let ulen = std::cmp::max(i, n);
            for j in (1..i).rev() {
                if last < w[(j - 1) as usize] && w[(j - 1) as usize] < wi {
                    last = w[(j - 1) as usize];
                    let mut vec = vec![0; ulen as usize];
                    vec[0..n as usize].copy_from_slice(&w[..n as usize]);
                    for t in n..ulen {
                        vec[t as usize] = (t + 1) as i32;
                    }
                    vec[(j - 1) as usize] = wi;
                    vec[(i - 1) as usize] = last;
                    add(vec, -c);
                }
            }
        } else {
            let mut vec = vec![0; i as usize];
            vec[0..n as usize].copy_from_slice(&w[..n as usize]);
            for t in n..(i - 2) {
                vec[t as usize] = (t + 1) as i32;
            }
            vec[(i - 2) as usize] = i as i32;
            vec[(i - 1) as usize] = i as i32 - 1;
            add(vec, -c);
        }

        if i > n {
            let mut vec = vec![0; (i + 1) as usize];
            vec[0..n as usize].copy_from_slice(&w[..n as usize]);
            for t in n..i {
                vec[t as usize] = (t + 1) as i32;
            }
            vec[(i - 1) as usize] = i as i32 + 1;
            vec[i as usize] = i as i32;
            add(vec, c);
        } else {
            let mut last = i32::MAX;
            for j in (i + 1)..=n {
                if wi < w[(j - 1) as usize] && w[(j - 1) as usize] < last {
                    last = w[(j - 1) as usize];
                    let mut vec = w[..n as usize].to_vec();
                    vec[(i - 1) as usize] = last;
                    vec[(j - 1) as usize] = wi;
                    add(vec, c);
                }
            }
            if last > n as i32 && (n as i32) < rank {
                let mut vec = vec![0; (n + 1) as usize];
                vec[0..n as usize].copy_from_slice(&w[..n as usize]);
                vec[(i - 1) as usize] = n as i32 + 1;
                vec[n as usize] = wi;
                add(vec, c);
            }
        }
    }
}

struct Poly {
    key: IntVector,
    val: i32,
}

pub fn mult_poly_schubert(poly: &mut LinearCombination, perm: &mut IntVector, mut rank: i32) {
    let n = poly.map.len();
    if n == 0 {
        poly.clear();
        return;
    }

    if rank == 0 {
        rank = i32::MAX;
    }

    let mut p = Vec::with_capacity(n as usize);
    let mut maxvar = 0;
    for kv in poly.map.drain() {
        let mut xx = kv.0.ptr;
        let mut j = xx.length;
        while j > 0 && xx[(j - 1) as usize] == 0 {
            j -= 1;
        }
        xx.length = j;
        if maxvar < j {
            maxvar = j;
        }
        p.push(Poly { key: xx, val: kv.1 });
    }
    debug_assert!(p.len() == n as usize);

    let svlen = perm.length;
    perm.length = perm_group(&perm[..]) as u32;
    _mult_ps(&mut p[..], n as u32, maxvar, perm, rank, poly);
    perm.length = svlen;
}

fn _mult_ps(
    poly: &mut [Poly],
    n: u32,
    maxvar: u32,
    perm: &IntVector,
    rank: i32,
    res: &mut LinearCombination,
) {
    if maxvar == 0 {
        let w = perm.clone();
        let hash = iv_hash(&w[..]);
        ivlc_add_element(res, poly[0].val, w, hash, LC_FREE_ZERO);
        return;
    }

    let mut mv0 = 0;
    let mut mv1 = 0;
    let mut j = 0;
    for i in 0..n {
        let xx = &mut poly[i as usize].key;
        let mut lnxx = xx.length;
        if lnxx < maxvar {
            if mv0 < lnxx {
                mv0 = lnxx;
            }
        } else {
            xx[(maxvar - 1) as usize] -= 1;
            while lnxx > 0 && xx[(lnxx - 1) as usize] == 0 {
                lnxx -= 1;
            }
            xx.length = lnxx;
            if mv1 < lnxx {
                mv1 = lnxx;
            }
            poly.swap(i as usize, j as usize);
            j += 1;
        }
    }

    let mut res1 = ivlc_new_default();
    _mult_ps(poly, j, mv1, perm, rank, &mut res1);
    _monk_add(maxvar, &res1, rank, res);

    if j < n {
        _mult_ps(&mut poly[j as usize..], n - j, mv0, perm, rank, res);
    }
}

pub fn mult_schubert(w1: &mut IntVector, w2: &mut IntVector, mut rank: i32) -> LinearCombination {
    let w1len = perm_length(&w1[..]);
    let w2len = perm_length(&w2[..]);
    let (w1, w2, w1len, w2len) = if w1len <= w2len {
        (w1, w2, w1len, w2len)
    } else {
        (w2, w1, w2len, w1len)
    };

    let svlen1 = w1.length;
    let svlen2 = w2.length;
    w1.length = perm_group(&w1[..]) as u32;
    w2.length = perm_group(&w2[..]) as u32;

    if rank == 0 {
        rank = i32::MAX;
    } else if 2 * (w1len + w2len) > rank * (rank - 1) || bruhat_zero(&w1[..], &w2[..], rank) {
        w1.length = svlen1;
        w2.length = svlen2;
        return ivlc_new_default();
    }

    let mut lc = trans(&w1[..], 0);
    mult_poly_schubert(&mut lc, w2, rank);

    w1.length = svlen1;
    w2.length = svlen2;
    lc
}

pub fn mult_schubert_str(str1: &IntVector, str2: &IntVector) -> Option<LinearCombination> {
    debug_assert!(str_iscompat(&str1[..], &str2[..]));

    // drop dv, w1, w2
    let dv = match str2dimvec(str1) {
        None => {
            return None;
        }
        Some(v) => v,
    };
    let mut w1 = string2perm(str1);
    let mut w2 = string2perm(str2);

    let len = w1.length as i32;
    let lc = mult_schubert(&mut w1, &mut w2, len);

    drop(w1);
    drop(w2);

    let mut res = ivlc_new_default();
    for kv in lc.map.iter() {
        let str = perm2string(&(kv.0.ptr)[..], &dv[..]);
        let hash = iv_hash(&str[..]);
        _ivlc_insert(&mut res, str, hash, *kv.1);
    }

    Some(res)
}
