use super::ivector::{iv_free, iv_hash, iv_new, IntVector};
use super::ivlincomb::{
    _ivlc_insert, ivlc_add_element, ivlc_add_multiple, ivlc_free, ivlc_free_all, ivlc_new_default,
    ivlc_reset, LinearCombination, LinearCombinationIter, LC_FREE_ZERO,
};
use super::perm::{
    bruhat_zero, perm2string, perm_group, perm_length, str2dimvec, str_iscompat, string2perm,
};

pub fn trans(w: &[i32], vars: i32) -> *mut LinearCombination {
    let res = unsafe { &mut *ivlc_new_default() };
    _trans(&mut w.to_vec()[..], vars, res);
    res
}

fn _trans(w: &mut [i32], mut vars: i32, res: &mut LinearCombination) {
    ivlc_reset(res);

    let n = perm_group(w);

    let mut r = n - 1;
    while r > 0 && w[(r - 1) as usize] < w[r as usize] {
        r -= 1;
    }
    if r <= 0 {
        let xx = unsafe { &mut *iv_new(std::cmp::max(vars as u32, 1)) };
        _ivlc_insert(res, xx, iv_hash(&xx[..]), 1);
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

    let tmp = unsafe { &mut *trans(w, vars) };
    for kv in LinearCombinationIter::from(tmp as *const _) {
        let xx = unsafe { &mut *kv.key };
        xx[(r - 1) as usize] += 1;
        _ivlc_insert(res, xx, iv_hash(&xx[..]), kv.value);
    }

    let mut last = 0;
    let vr = w[(r - 1) as usize];
    for i in (1..r).rev() {
        let vi = w[(i - 1) as usize];
        if last < vi && vi < vr {
            last = vi;
            w[(i - 1) as usize] = vr;
            w[(r - 1) as usize] = vi;
            _trans(w, vars, tmp);
            ivlc_add_multiple(res, 1, tmp, LC_FREE_ZERO);
            w[(i - 1) as usize] = vi;
        }
    }

    w[(s - 1) as usize] = ws;
    w[(r - 1) as usize] = wr;
    ivlc_free(tmp);
}

#[allow(clippy::many_single_char_names)]
fn _monk_add(i: u32, slc: &LinearCombination, rank: i32, res: &mut LinearCombination) {
    let mut add = |u: Vec<i32>, c: i32| {
        let u = unsafe { &mut *IntVector::from_vec(u) };
        ivlc_add_element(res, c, u, iv_hash(&u[..]), LC_FREE_ZERO)
    };
    for kv in LinearCombinationIter::from(slc as *const _) {
        let w = unsafe { &(*kv.key)[..] };
        let c = kv.value;
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
                    let mut u = vec![0; ulen as usize];
                    u[0..n as usize].copy_from_slice(&w[..n as usize]);
                    for t in n..ulen {
                        u[t as usize] = (t + 1) as i32;
                    }
                    u[(j - 1) as usize] = wi;
                    u[(i - 1) as usize] = last;
                    add(u, -c);
                }
            }
        } else {
            let mut u = vec![0; i as usize];
            u[0..n as usize].copy_from_slice(&w[..n as usize]);
            for t in n..(i - 2) {
                u[t as usize] = (t + 1) as i32;
            }
            u[(i - 2) as usize] = i as i32;
            u[(i - 1) as usize] = i as i32 - 1;
            add(u, -c);
        }

        if i > n {
            let mut u = vec![0; (i + 1) as usize];
            u[0..n as usize].copy_from_slice(&w[..n as usize]);
            for t in n..i {
                u[t as usize] = (t + 1) as i32;
            }
            u[(i - 1) as usize] = i as i32 + 1;
            u[i as usize] = i as i32;
            add(u, c);
        } else {
            let mut last = i32::MAX;
            for j in (i + 1)..=n {
                if wi < w[(j - 1) as usize] && w[(j - 1) as usize] < last {
                    last = w[(j - 1) as usize];
                    let mut u = w[..n as usize].to_vec();
                    u[(i - 1) as usize] = last;
                    u[(j - 1) as usize] = wi;
                    add(u, c);
                }
            }
            if last > n as i32 && (n as i32) < rank {
                let mut u = vec![0; (n + 1) as usize];
                u[0..n as usize].copy_from_slice(&w[..n as usize]);
                u[(i - 1) as usize] = n as i32 + 1;
                u[n as usize] = wi;
                add(u, c);
            }
        }
    }
}

struct Poly {
    key: *mut IntVector,
    val: i32,
}

pub fn mult_poly_schubert(
    poly: &mut LinearCombination,
    perm: &mut IntVector,
    mut rank: i32,
) -> *mut LinearCombination {
    let n = poly.card;
    if n == 0 {
        ivlc_free_all(poly);
        return poly;
    }

    if rank == 0 {
        rank = i32::MAX;
    }

    let mut p = Vec::with_capacity(n as usize);
    let mut maxvar = 0;
    for kv in LinearCombinationIter::from(poly as *const _) {
        let xx = unsafe { &mut *kv.key };
        let mut j = xx.length;
        while j > 0 && xx[(j - 1) as usize] == 0 {
            j -= 1;
        }
        xx.length = j;
        if maxvar < j {
            maxvar = j;
        }
        p.push(Poly {
            key: kv.key,
            val: kv.value,
        });
    }
    debug_assert!(p.len() == n as usize);
    ivlc_reset(poly);

    let svlen = perm.length;
    perm.length = perm_group(&perm[..]) as u32;
    _mult_ps(&mut p[..], n, maxvar, perm, rank, poly);
    perm.length = svlen;

    poly
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
        let w = unsafe { &mut *IntVector::from_vec((&perm[..]).to_vec()) };
        ivlc_add_element(res, poly[0].val, w, iv_hash(&w[..]), LC_FREE_ZERO);
        return;
    }

    let mut mv0 = 0;
    let mut mv1 = 0;
    let mut j = 0;
    for i in 0..n {
        let xx = unsafe { &mut *poly[i as usize].key };
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
            let t = poly[i as usize].key;
            poly[i as usize].key = poly[j as usize].key;
            poly[j as usize].key = t;
            let t = poly[i as usize].val;
            poly[i as usize].val = poly[j as usize].val;
            poly[j as usize].val = t;
            j += 1;
        }
    }

    let res1 = unsafe { &mut *ivlc_new_default() };
    _mult_ps(poly, j, mv1, perm, rank, res1);
    _monk_add(maxvar, res1, rank, res);

    if j < n {
        _mult_ps(&mut poly[j as usize..], n - j, mv0, perm, rank, res);
    }
    ivlc_free_all(res1);
}

pub fn mult_schubert(
    w1: &mut IntVector,
    w2: &mut IntVector,
    mut rank: i32,
) -> *mut LinearCombination {
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

    let lc = mult_poly_schubert(unsafe { &mut *trans(&w1[..], 0) }, w2, rank);

    w1.length = svlen1;
    w2.length = svlen2;
    lc
}

struct IntVectorDisposed {
    p: IntVector,
}

impl Drop for IntVectorDisposed {
    fn drop(&mut self) {
        iv_free(&mut self.p)
    }
}

pub fn mult_schubert_str(str1: &IntVector, str2: &IntVector) -> *mut LinearCombination {
    debug_assert!(str_iscompat(&str1[..], &str2[..]));

    // drop dv, w1, w2
    let dv = IntVectorDisposed {
        p: match str2dimvec(str1) {
            None => {
                return std::ptr::null_mut();
            }
            Some(v) => v,
        },
    };
    let mut w1 = IntVectorDisposed {
        p: string2perm(str1),
    };
    let mut w2 = IntVectorDisposed {
        p: string2perm(str2),
    };

    let len = w1.p.length as i32;
    let lc = mult_schubert(&mut w1.p, &mut w2.p, len);

    drop(w1);
    drop(w2);

    let res = unsafe { &mut *ivlc_new_default() };
    for kv in LinearCombinationIter::from(lc as *const _) {
        let str = unsafe { &mut *perm2string(&(*kv.key)[..], &dv.p[..]) };
        _ivlc_insert(res, str, iv_hash(&str[..]), kv.value);
    }

    ivlc_free_all(lc);
    res
}