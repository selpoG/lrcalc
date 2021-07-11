use super::{
    ivector::{iv_sum, IntVector},
    part::{part_entry, part_length, part_leq, part_valid},
};

#[derive(Clone)]
struct LRCoefBox {
    /// integer in box of skew tableau
    value: i32,
    /// upper bound for integer in box
    max: i32,
    /// index of box above
    north: i32,
    /// index of box to the right
    east: i32,
    /// number of available integers larger than value
    se_supply: i32,
    /// number of boxes to the right and strictly below
    se_sz: i32,
    /// number of boxes strictly to the left in same row
    west_sz: i32,
    /// make size a power of 2, improves speed in x86_64
    _padding: i32,
}

#[derive(Clone)]
struct LRCoefContent {
    /// number of boxes containing a given integer
    cont: i32,
    /// total supply of given integer
    supply: i32,
}

fn lrcoef_new_content(mu: &IntVector) -> Vec<LRCoefContent> {
    debug_assert!(part_valid(&mu[..]));
    debug_assert!(part_length(&mu[..]) > 0);

    let n = part_length(&mu[..]) as usize;
    let mu = &mu[..];
    let mut res = vec![LRCoefContent { cont: 0, supply: 0 }; n + 1];
    res[0] = LRCoefContent {
        cont: mu[0],
        supply: mu[0],
    };
    for i in 0..n {
        res[i + 1] = LRCoefContent {
            cont: 0,
            supply: mu[i],
        }
    }
    res
}

fn lrcoef_new_skewtab(nu: &IntVector, la: &IntVector, max_value: i32) -> Vec<LRCoefBox> {
    debug_assert!(part_valid(&nu[..]));
    debug_assert!(part_valid(&la[..]));
    debug_assert!(part_leq(la, nu));
    debug_assert!(part_entry(&nu[..], 0) > 0);

    let n = (iv_sum(nu) - iv_sum(la)) as usize;
    let nu = &nu[..];
    let la = &la[..];
    let mut array = vec![
        LRCoefBox {
            value: 0,
            north: 0,
            max: 0,
            east: 0,
            se_supply: 0,
            se_sz: 0,
            west_sz: 0,
            _padding: 0
        };
        n + 2
    ];

    let mut pos = n as i32;
    for r in (0..nu.len()).rev() {
        let nu_0 = if r == 0 { nu[0] } else { nu[r - 1] };
        let la_0 = if r == 0 {
            nu[0]
        } else {
            part_entry(la, r as i32 - 1)
        };
        let nu_r = nu[r];
        let la_r = part_entry(la, r as i32);
        let nu_1 = part_entry(nu, r as i32 + 1);
        for c in la_r..nu_r {
            pos -= 1;
            let mut b = array[pos as usize].clone();
            b.north = if la_0 <= c && c < nu_0 {
                pos - nu_r + la_0
            } else {
                n as i32
            };
            b.east = if c + 1 < nu_r { pos - 1 } else { n as i32 + 1 };
            b.west_sz = c - la_r;
            if c >= nu_1 {
                b.max = max_value;
                b.se_sz = 0;
            } else {
                let below = pos + nu_1 - la_r;
                b.max = array[below as usize].max - 1;
                b.se_sz = array[below as usize].se_sz + nu_1 - c;
            }
            array[pos as usize] = b;
        }
    }
    array[n].value = 0;
    array[n + 1].value = max_value;
    array[n + 1].se_supply = 0;
    array
}

/// This is a low level function called from schur_lrcoef().
pub(crate) fn lrcoef_count(outer: &IntVector, inner: &IntVector, content: &IntVector) -> i64 {
    debug_assert!(iv_sum(outer) == iv_sum(inner) + iv_sum(content));
    debug_assert!(iv_sum(content) > 1);

    let mut t =
        lrcoef_new_skewtab(outer, inner, part_length(&content[..]) as i32).into_boxed_slice();
    let mut cont = lrcoef_new_content(content).into_boxed_slice();

    let n = iv_sum(content);
    let mut pos = 0;
    let mut b = 0;
    let mut b_ref = &t[b];
    let mut coef = 0i64;
    let mut above = t[b_ref.north as usize].value;
    let mut x = 1;
    let mut se_supply = n - cont[1].supply;

    loop {
        while x > above
            && (cont[x as usize].cont == cont[x as usize].supply
                || cont[x as usize].cont == cont[(x - 1) as usize].cont)
        {
            se_supply += cont[x as usize].supply - cont[x as usize].cont;
            x -= 1;
        }

        if x == above || n - pos - se_supply <= b_ref.west_sz {
            pos -= 1;
            if pos < 0 {
                break;
            }
            b -= 1;
            b_ref = &t[b];
            se_supply = b_ref.se_supply;
            above = t[b_ref.north as usize].value;
            x = b_ref.value;
            cont[x as usize].cont -= 1;
            se_supply += cont[x as usize].supply - cont[x as usize].cont;
            x -= 1;
        } else if pos + 1 < n {
            t[b].se_supply = se_supply;
            t[b].value = x;
            cont[x as usize].cont += 1;
            pos += 1;
            b += 1;
            b_ref = &t[b];
            se_supply = t[b_ref.east as usize].se_supply;
            x = t[b_ref.east as usize].value;
            above = t[b_ref.north as usize].value;
            while x > b_ref.max {
                se_supply += cont[x as usize].supply - cont[x as usize].cont;
                x -= 1;
            }
            while x > above && se_supply < b_ref.se_sz {
                se_supply += cont[x as usize].supply - cont[x as usize].cont;
                x -= 1;
            }
        } else {
            coef += 1;
            pos -= 1;
            b -= 1;
            b_ref = &t[b];
            se_supply = b_ref.se_supply;
            above = t[b_ref.north as usize].value;
            x = b_ref.value;
            cont[x as usize].cont -= 1;
            se_supply += cont[x as usize].supply - cont[x as usize].cont;
            x -= 1;
        }
    }
    coef
}
