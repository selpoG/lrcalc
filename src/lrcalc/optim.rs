use super::helper::{
    ivector::IntVector,
    part::{part_entry, part_length, part_leq, part_valid},
};

pub struct SkewShape {
    pub outer: Option<IntVector>,
    pub inner: Option<IntVector>,
    pub cont: Option<IntVector>,
    pub sign: i32,
}

pub(crate) fn _sksh_print(outer: &[i32], inner: Option<&[i32]>, cont: Option<&[i32]>) {
    let mut len = part_length(outer);
    let mut ilen = inner.map(|v| v.len()).unwrap_or(0) as u32;
    let clen = cont.map(|v| part_length(v)).unwrap_or(0);
    if len <= ilen {
        while len > 0 && inner.unwrap()[(len - 1) as usize] == outer[(len - 1) as usize] {
            len -= 1;
        }
        ilen = len;
    }
    let mut r0 = 0;
    while r0 < ilen && inner.unwrap()[r0 as usize] == outer[r0 as usize] {
        r0 += 1;
    }
    let ss_left = if len == 0 || ilen < len {
        0
    } else {
        inner.unwrap()[(len - 1) as usize]
    };
    let ss_right = if len == 0 { 0 } else { outer[0] };

    for r in 0..clen {
        println!(
            "{}{}",
            " ".repeat((ss_right - ss_left) as usize),
            "c".repeat(cont.unwrap()[r as usize] as usize)
        );
    }

    for r in r0..len {
        let innr = if r < ilen {
            inner.unwrap()[r as usize]
        } else {
            0
        };
        let outr = outer[r as usize];
        println!(
            "{}{}",
            " ".repeat(innr as usize),
            "s".repeat((outr - innr) as usize)
        );
    }
}

/// Find optimal shape for product of Schur functions.
///
/// 1) Let outer0 be outer minus all rows of size maxcols and all columns of size maxrows.
///
/// 2) Let content0 be content minus all rows of size maxcols and all columns of size maxrows.
///
/// 3) New outer should be smaller of outer0, content0.
///
/// 4) New content should be larger shape, plus removed rows and columns.
pub(crate) fn optim_mult(
    sh1: &IntVector,
    sh2: Option<&IntVector>,
    maxrows: i32,
    maxcols: i32,
) -> SkewShape {
    /* DEBUG: Check valid input. */
    debug_assert!(part_valid(&sh1[..]), "sh1 is not a partition");
    if sh2.is_some() {
        debug_assert!(part_valid(&sh2.unwrap()[..]), "sh1 is not a partition");
    }

    let v1 = &sh1[..];
    let v2: &[i32] = match sh2 {
        None => &[], // this must not be accessed!
        Some(v) => &v[..],
    };

    /* Find length and width of shapes. */
    let len1 = part_length(v1) as i32;
    let sh10 = if len1 != 0 { v1[0] } else { 0 };
    let len2 = if sh2.is_some() {
        part_length(v2) as i32
    } else {
        0
    };
    let sh20 = if len2 != 0 { v2[0] } else { 0 };

    /* Indicate empty result. */
    let mut ss = SkewShape {
        outer: None,
        inner: None,
        cont: None,
        sign: 0,
    };

    /* Empty result? */
    if maxrows >= 0 && (len1 > maxrows || len2 > maxrows) {
        return ss;
    }
    if maxcols >= 0 && (sh10 > maxcols || sh20 > maxcols) {
        return ss;
    }
    if maxrows >= 0 && maxcols >= 0 {
        let mut r = if len1 + len2 < maxrows {
            len2
        } else {
            maxrows - len1
        };
        while r < len2 {
            if v1[(maxrows - r - 1) as usize] + v2[r as usize] > maxcols {
                return ss;
            }
            r += 1;
        }
    }

    /* Number of full rows and columns in shapes. */
    let fc1 = if len1 == maxrows && len1 > 0 {
        v1[(len1 - 1) as usize]
    } else {
        0
    };
    let mut fr1 = 0;
    while fr1 < len1 && v1[fr1 as usize] == maxcols {
        fr1 += 1;
    }
    let fc2 = if len2 == maxrows && len2 > 0 {
        v2[(len2 - 1) as usize]
    } else {
        0
    };
    let mut fr2 = 0;
    while fr2 < len2 && v2[fr2 as usize] == maxcols {
        fr2 += 1;
    }

    /* Find # boxes after removing full rows and columns. */
    let mut sz1 = (fr1 - len1) * fc1;
    for r in fr1..len1 {
        sz1 += v1[r as usize]
    }
    let mut sz2 = (fr2 - len2) * fc2;
    for r in fr2..len2 {
        sz2 += v2[r as usize]
    }

    struct Obj<'a> {
        v: &'a [i32],
        len: i32,
        fc: i32,
        fr: i32,
    }
    let mut o1 = &Obj {
        v: v1,
        len: len1,
        fc: fc1,
        fr: fr1,
    };
    let mut o2 = &Obj {
        v: v2,
        len: len2,
        fc: fc2,
        fr: fr2,
    };
    /* sh2 should be largest partition. */
    if sz1 > sz2 {
        std::mem::swap(&mut o1, &mut o2);
    }

    /* Remove full rows and columns from sh1. */
    let mut out = vec![0; (o1.len - o1.fr) as usize];
    for (r, x) in out.iter_mut().enumerate() {
        *x = o1.v[(o1.fr as usize + r) as usize] - o1.fc
    }

    /* Add full rows and columns to sh2. */
    let clen = if o1.fc + o2.fc > 0 {
        maxrows
    } else {
        o2.len + o1.fr
    };
    let mut cont = vec![0; clen as usize];
    for r in 0..o1.fr {
        cont[r as usize] = maxcols
    }
    for r in 0..o2.len {
        cont[(o1.fr + r) as usize] = o2.v[r as usize] + o1.fc
    }
    for r in (o2.len + o1.fr)..clen {
        cont[r as usize] = o1.fc
    }

    ss.outer = Some(out.into());
    ss.cont = Some(cont.into());
    ss.sign = 1;
    ss
}

/// Find optimal shape for fusion product.
pub(crate) fn optim_fusion(sh1: &IntVector, sh2: &IntVector, rows: i32, level: i32) -> SkewShape {
    /* DEBUG: Check valid input. */
    debug_assert!(part_valid(&sh1[..]));
    debug_assert!(part_valid(&sh2[..]));
    debug_assert!(part_entry(&sh1[..], 0) - part_entry(&sh1[..], rows - 1) <= level);
    debug_assert!(part_entry(&sh2[..], 0) - part_entry(&sh2[..], rows - 1) <= level);

    /* Empty result? */
    let mut ss = SkewShape {
        outer: None,
        inner: None,
        cont: None,
        sign: 0,
    };
    if part_length(&sh1[..]) as i32 > rows || part_length(&sh2[..]) as i32 > rows {
        return ss;
    }

    /* Find Seidel shift that results in smallest LHS partition. */
    let mut d1 = 0;
    let mut d2 = 0;
    let mut s1 = rows * level;
    let mut s2 = s1;
    for d in 1..=rows {
        let s = (rows - d) * level - rows * part_entry(&sh1[..], d - 1);
        if s < s1 {
            d1 = d;
            s1 = s;
        }
        let s = (rows - d) * level - rows * part_entry(&sh2[..], d - 1);
        if s < s2 {
            d2 = d;
            s2 = s;
        }
    }
    let (d, sh1, sh2) = if s1 <= s2 {
        (d1, sh1, sh2)
    } else {
        (d2, sh2, sh1)
    };
    let sh1d = part_entry(&sh1[..], d - 1);

    /* Create shifted partitions. */
    let mut nsh1 = vec![0; rows as usize];
    for i in 0..(rows - d) {
        nsh1[i as usize] = part_entry(&sh1[..], d + i) - sh1d + level;
    }
    for i in 0..d {
        nsh1[(rows - d + i) as usize] = part_entry(&sh1[..], i) - sh1d;
    }
    let mut nsh2 = vec![0; rows as usize];
    for i in 0..d {
        nsh2[i as usize] = part_entry(&sh2[..], rows - d + i) + sh1d;
    }
    for i in 0..(rows - d) {
        nsh2[(d + i) as usize] = part_entry(&sh2[..], i) + sh1d - level;
    }

    ss.outer = Some(nsh1.into());
    ss.cont = Some(nsh2.into());
    ss.sign = 1;
    ss
}

struct PartialShape {
    /// Build skew shape out/inn.
    inn: Vec<i32>,
    out: Vec<i32>,
    /// Max column height.
    rows: i32,
    /// Left edge of skew shape is (top,col)...(bot-1,col).
    top: i32,
    bot: i32,
    col: i32,
}

struct AddInfo {
    c: i32,
    rt: i32,
    rb: i32,
}

impl PartialShape {
    fn new(inn: Vec<i32>, out: Vec<i32>, rows: i32) -> PartialShape {
        PartialShape {
            inn,
            out,
            rows,
            top: 0,
            bot: 0,
            col: 0,
        }
    }
    fn add_comp(&mut self, out0: &[i32], inn0: Option<&[i32]>, a0: AddInfo, a1: AddInfo) {
        let mut x = self.top + self.rows + a1.rt - a1.rb;
        if x > self.bot {
            x = self.bot;
        }
        let y1 = x + a1.rb - a1.rt;
        let z = y1 + a0.rb - a1.rb;

        for r in self.bot..y1 {
            self.out[r as usize] = self.col;
        }
        for r in y1..z {
            let c = out0[(r - x + a1.rt) as usize];
            self.out[r as usize] = self.col + c - a1.c;
        }

        let len0 = inn0.map(|v| v.len() as i32).unwrap_or(0);
        let y0 = x + a0.rt - a1.rt;
        for r in x..y0 {
            let ra = r - x + a1.rt;
            let c = if ra < len0 {
                inn0.unwrap()[ra as usize]
            } else {
                0
            };
            self.inn[r as usize] = self.col + c - a1.c;
        }
        for r in y0..z {
            self.inn[r as usize] = self.col - a1.c + a0.c;
        }

        self.col -= a1.c - a0.c;
        self.top = y0;
        self.bot = z;
    }
}

/// Find optimal shape for skew Schur function.
///
/// 1) Check if outer/inner or content has column of height > maxrows.
///
/// 2) Remove columns of height maxrows from outer/inner and content.
///
/// 3) Separate outer/inner into independent components.
///
/// 4) New content should be largest (anti) partition shaped component
///    plus all columns of height maxrows.
pub(crate) fn optim_skew(
    outer: &IntVector,
    inner: Option<&IntVector>,
    content: Option<&IntVector>,
    mut maxrows: i32,
) -> SkewShape {
    /* Handle case in other function. */
    let inner = match inner {
        None => {
            return optim_mult(outer, content, maxrows, -1);
        }
        Some(inner) => inner,
    };

    /* DEBUG: Check valid input. */
    debug_assert!(part_valid(&outer[..]));
    debug_assert!(part_valid(&inner[..]));
    if content.is_some() {
        debug_assert!(part_valid(&content.unwrap()[..]));
    }

    /* Indicate empty result. */
    let mut ss = SkewShape {
        outer: None,
        inner: None,
        cont: None,
        sign: 0,
    };
    if !part_leq(inner, outer) {
        return ss;
    }

    /* Find range of non-empty rows in outer/inner. */
    let mut row_bound = part_length(&outer[..]) as i32;
    let outer = &outer[..];
    let inner = &inner[..];
    let mut ilen = inner.len() as i32;
    if row_bound <= ilen {
        while row_bound > 0 && inner[(row_bound - 1) as usize] == outer[(row_bound - 1) as usize] {
            row_bound -= 1;
        }
        ilen = row_bound;
    }
    let mut row_first = 0;
    while row_first < ilen && inner[row_first as usize] == outer[row_first as usize] {
        row_first += 1;
    }
    let row_span = row_bound - row_first;

    /* Bound number of rows in content of LR tableaux. */
    let (content, mut clen) = match content {
        None => (None, 0),
        Some(v) => (Some(&v[..]), part_length(&v[..]) as i32),
    };
    if maxrows >= 0 && clen > maxrows {
        return ss;
    }
    /* FIXME: Prove slen is large enough!!! */
    let slen = 2 * row_span + clen;
    if maxrows < 0 {
        maxrows = slen + 1;
    }

    /* Allocate new skew shape. */
    let out = vec![0; slen as usize];
    let inn = vec![0; slen as usize];

    /* Allocate and copy content. */
    let mut cont = vec![0; std::cmp::max(clen, row_span) as usize];
    let mut cont_size = 0;
    for r in (0..clen).rev() {
        cont[r as usize] = content.unwrap()[r as usize];
        cont_size += content.unwrap()[r as usize];
    }

    /* Empty shape outer/inner ? */
    if row_bound == 0 {
        ss.outer = Some(out.into());
        ss.inner = Some(inn.into());
        ss.cont = Some(cont.into());
        ss.outer.as_mut().unwrap().length = 0;
        ss.inner.as_mut().unwrap().length = 0;
        ss.cont.as_mut().unwrap().length = clen as u32;
        ss.sign = 1;
        return ss;
    }

    /* Number of columns of size maxrows. */
    let mut full_cols = 0;
    if clen == maxrows && maxrows > 0 {
        full_cols = cont[(clen - 1) as usize];
        for r in (0..clen).rev() {
            cont[r as usize] -= full_cols;
        }
        cont_size -= full_cols * clen;
    }

    /* Find component with upper-right (r2t,c2-1), lower-right (r2b-1,c2-1). */
    let mut c2 = outer[row_first as usize];
    let mut r2t = row_first;
    let mut r2b = row_first;
    while r2b < row_bound && c2 <= outer[r2b as usize] {
        r2b += 1;
    }

    /* Find component with upper-left (r1t,c1), lower-left (r1b-1,c1). */
    let mut c1 = c2;
    let mut r0t = r2t;
    let mut r0b = r2b;

    /* Skew shape structure to pass to add_comp. */
    let mut ps = PartialShape::new(inn, out, maxrows);

    c1 -= 1;
    while c1 >= 0 {
        /* Find row range for column c1-1. */
        let r1t = r0t;
        let r1b = r0b;
        if c1 == 0 {
            r0t = row_bound;
            r0b = row_bound;
        }
        while r0b < row_bound && c1 <= outer[r0b as usize] {
            r0b += 1;
        }
        while r0t < inner.len() as i32 && c1 <= inner[r0t as usize] {
            r0t += 1;
        }

        /* No new component? */
        if r0t < r1b && r0b - r1t < maxrows {
            c1 -= 1;
            continue;
        }

        /* Single column too high? */
        if c1 == c2 - 1 && r1b - r1t > maxrows {
            return ss;
        }

        /* Single column of full height? */
        if c1 == c2 - 1 && r1b - r1t == maxrows {
            full_cols += 1;
            c2 = c1;
            r2t = r0t;
            r2b = r0b;
            c1 -= 1;
            continue;
        }

        /* Find size of component. */
        let mut comp_size = 0;
        for r in r2t..r1b {
            let mut a = if r < inner.len() as i32 {
                inner[r as usize]
            } else {
                0
            };
            if a < c1 {
                a = c1;
            }
            let mut b = outer[r as usize];
            if b > c2 {
                b = c2;
            }
            comp_size += b - a;
        }

        if (r1t == r2t || r1b == r2b) && 0 < cont_size && cont_size < comp_size {
            /* Add content as component. */
            let mut r = 1;
            let c = cont[0];
            while r < clen && cont[r as usize] == c {
                r += 1;
            }
            ps.add_comp(
                &cont[..],
                None,
                AddInfo {
                    c: 0,
                    rt: 0,
                    rb: clen,
                },
                AddInfo { c, rt: 0, rb: r },
            );
        }

        if r1t == r2t && comp_size > cont_size {
            /* Component of larger partition shape. */
            clen = r1b - r1t;
            for r in r1t..r2b {
                cont[(r - r1t) as usize] = c2 - c1;
            }
            for r in r2b..r1b {
                cont[(r - r1t) as usize] = outer[r as usize] - c1;
            }
            cont_size = comp_size;
        } else if r1b == r2b && comp_size > cont_size {
            /* Component of larger anti-partition shape. */
            clen = r2b - r2t;
            for r in (r1t..r2b).rev() {
                cont[(r2b - 1 - r) as usize] = c2 - c1;
            }
            for r in (r2t..r1t).rev() {
                cont[(r2b - 1 - r) as usize] = c2 - inner[r as usize];
            }
            cont_size = comp_size;
        } else if comp_size > 0 {
            ps.add_comp(
                outer,
                Some(inner),
                AddInfo {
                    c: c1,
                    rt: r1t,
                    rb: r1b,
                },
                AddInfo {
                    c: c2,
                    rt: r2t,
                    rb: r2b,
                },
            );
        }

        c2 = c1;
        r2t = r0t;
        r2b = r0b;
        c1 -= 1;
    }

    if full_cols != 0 {
        for r in 0..clen {
            cont[r as usize] += full_cols;
        }
        for r in clen..maxrows {
            cont[r as usize] = full_cols;
        }
        clen = maxrows;
    }
    cont.truncate(clen as usize);

    ps.out.truncate(ps.bot as usize);
    ps.inn.truncate(ps.bot as usize);
    for r in (0..ps.bot).rev() {
        ps.out[r as usize] -= ps.col;
        ps.inn[r as usize] -= ps.col;
    }

    ss.outer = Some(ps.out.into());
    ss.inner = Some(ps.inn.into());
    ss.cont = Some(cont.into());
    ss.sign = 1;
    ss
}

pub(crate) fn optim_coef(out: &IntVector, sh1: &IntVector, sh2: &IntVector) -> SkewShape {
    debug_assert!(part_valid(&out[..]));
    debug_assert!(part_valid(&sh1[..]));
    debug_assert!(part_valid(&sh2[..]));

    let mut ss = SkewShape {
        outer: None,
        inner: None,
        cont: None,
        sign: 0,
    };
    fn ret_coef_one(mut ss: SkewShape) -> SkewShape {
        ss.sign = 1;
        ss
    }

    let mut n = part_length(&out[..]) as i32;
    if n < sh1.length as i32 && sh1[n as usize] > 0 {
        return ss;
    }
    if n < sh2.length as i32 && sh2[n as usize] > 0 {
        return ss;
    }
    if n == 0 {
        return ret_coef_one(ss);
    }

    let mut nu = vec![0; n as usize];
    let mut la = vec![0; n as usize];
    let mut mu = vec![0; n as usize];

    let mut sum = 0;
    for r in (0..n).rev() {
        nu[r as usize] = out[r as usize];
        sum += nu[r as usize];
    }

    let mut n_la = n;
    while n_la > sh1.length as i32 {
        la[(n_la - 1) as usize] = 0;
        n_la -= 1;
    }
    while n_la > 0 && sh1[(n_la - 1) as usize] == 0 {
        la[(n_la - 1) as usize] = 0;
        n_la -= 1;
    }
    for r in (0..n_la).rev() {
        let x = sh1[r as usize];
        la[r as usize] = x;
        if nu[r as usize] < x {
            return ss;
        }
        sum -= la[r as usize];
    }

    let mut n_mu = n;
    while n_mu > sh2.length as i32 {
        mu[(n_mu - 1) as usize] = 0;
        n_mu -= 1;
    }
    while n_mu > 0 && sh2[(n_mu - 1) as usize] == 0 {
        mu[(n_mu - 1) as usize] = 0;
        n_mu -= 1;
    }
    for r in (0..n_mu).rev() {
        let x = sh2[r as usize];
        mu[r as usize] = x;
        if nu[r as usize] < x {
            return ss;
        }
        sum -= mu[r as usize];
    }

    if sum != 0 {
        return ss;
    }

    let mut n0 = n + 1;
    let mut nu0 = 0;
    while n < n0 || nu[0] < nu0 {
        n0 = n;
        nu0 = nu[0];

        /* Horizontal compactification of nu/la. */
        let mu0 = mu[0];
        let mut lar1 = 0;
        let mut nur1 = 0;
        let mut r = n - 1;
        while r >= 0 {
            let lar = la[r as usize];
            let nur = nu[r as usize];
            if lar > nur1 || nur - lar1 > mu0 {
                break;
            }
            lar1 = lar;
            nur1 = nur;
            r -= 1;
        }
        let mut c = 0;
        while r >= 0 {
            let lar = la[r as usize];
            let nur = nu[r as usize];
            if nur - lar > mu0 {
                return ss;
            }
            let mut ca = nur - lar1 - mu0;
            if ca < lar - nur1 {
                ca = lar - nur1;
            }
            if ca > 0 {
                c += ca;
            }
            if nur - c < mu[r as usize] {
                return ss;
            }
            if nur == c {
                n = r;
                break;
            }
            la[r as usize] = lar - c;
            nu[r as usize] = nur - c;
            lar1 = lar;
            nur1 = nur;
            r -= 1;
        }

        /* Remove row of size mu[0] from nu/la. */
        r = 0;
        while r < n && nu[r as usize] - la[r as usize] < mu0 {
            r += 1;
        }
        if r < n {
            if nu[r as usize] - la[r as usize] > mu0 {
                return ss;
            }
            while r < n - 1 {
                la[r as usize] = la[(r + 1) as usize];
                nu[r as usize] = nu[(r + 1) as usize];
                r += 1;
            }
            for r in 0..(n - 1) {
                mu[r as usize] = mu[(r + 1) as usize];
            }
            n -= 1;
        }

        /* Horizontal compactification of nu/mu. */
        let la0 = la[0];
        let mut mur1 = 0;
        nur1 = 0;
        r = n - 1;
        while r >= 0 {
            let mur = mu[r as usize];
            let nur = nu[r as usize];
            if mur > nur1 || nur - mur1 > la0 {
                break;
            }
            mur1 = mur;
            nur1 = nur;
            r -= 1
        }
        c = 0;
        while r >= 0 {
            let mur = mu[r as usize];
            let nur = nu[r as usize];
            if nur - mur > la0 {
                return ss;
            }
            let mut ca = nur - mur1 - la0;
            if ca < mur - nur1 {
                ca = mur - nur1;
            }
            if ca > 0 {
                c += ca;
            }
            if nur - c < la[r as usize] {
                return ss;
            }
            if nur == c {
                n = r;
                break;
            }
            mu[r as usize] = mur - c;
            nu[r as usize] = nur - c;
            mur1 = mur;
            nur1 = nur;
            r -= 1;
        }

        /* Remove row of size la[0] from nu/la. */
        let mut r = 0;
        while r < n && nu[r as usize] - mu[r as usize] < la0 {
            r += 1;
        }
        if r < n {
            if nu[r as usize] - mu[r as usize] > la0 {
                return ss;
            }
            while r < n - 1 {
                mu[r as usize] = mu[(r + 1) as usize];
                nu[r as usize] = nu[(r + 1) as usize];
                r += 1
            }
            for r in 0..(n - 1) {
                la[r as usize] = la[(r + 1) as usize];
            }
            n -= 1;
        }

        /* Vertical compactification of nu/la. */
        if n < n_mu {
            n_mu = n;
        }
        while n_mu > 0 && mu[(n_mu - 1) as usize] == 0 {
            n_mu -= 1;
        }
        if n_mu == 0 {
            return ret_coef_one(ss);
        }
        r = 0;
        while r < n_mu && la[r as usize] < nu[r as usize] {
            r += 1;
        }
        while r < n && la[r as usize] < nu[r as usize] && nu[r as usize] < la[(r - n_mu) as usize] {
            r += 1;
        }
        if r < n {
            let mut i_nu = r;
            let mut s = if r > n_mu { r - n_mu } else { 0 };
            let mut i_la = s;
            while r < n && i_nu < n_mu {
                if la[r as usize] == nu[r as usize] {
                    la[r as usize] = -1;
                } else {
                    nu[i_nu as usize] = nu[r as usize];
                    if nu[i_nu as usize] < mu[i_nu as usize] {
                        return ss;
                    }
                    i_nu += 1;
                }
                r += 1
            }
            while r < n {
                if la[r as usize] == nu[r as usize] {
                    la[r as usize] = -1;
                    r += 1;
                    continue;
                }
                while la[s as usize] == -1 {
                    s += 1;
                }
                if la[s as usize] < nu[r as usize] {
                    return ss;
                }
                if la[s as usize] > nu[r as usize] {
                    la[i_la as usize] = la[s as usize];
                    i_la += 1;
                    nu[i_nu as usize] = nu[r as usize];
                    if nu[i_nu as usize] < mu[i_nu as usize] {
                        return ss;
                    }
                    i_nu += 1;
                }
                r += 1;
                s += 1;
            }
            while s < n {
                if la[s as usize] != -1 {
                    la[i_la as usize] = la[s as usize];
                    i_la += 1;
                }
                s += 1;
            }
            if i_nu < n && mu[i_nu as usize] > 0 {
                return ss;
            }
            n = i_nu;
        }

        /* Remove column of size len(mu) from nu/la. */
        r = n_mu;
        while r <= n && nu[(r - 1) as usize] <= la[(r - n_mu) as usize] {
            r += 1;
        }
        if r <= n {
            if r > n_mu && nu[(r - 1) as usize] > la[(r - n_mu - 1) as usize] {
                return ss;
            }
            if r < n && nu[r as usize] > la[(r - n_mu) as usize] {
                return ss;
            }
            for s in (0..(r - n_mu)).rev() {
                la[s as usize] -= 1;
            }
            for s in (0..n_mu).rev() {
                mu[s as usize] -= 1;
            }
            for s in 0..r {
                nu[s as usize] -= 1;
                if nu[s as usize] == 0 {
                    n = s;
                    break;
                }
            }
        }

        /* Vertical compactification of nu/mu. */
        if n < n_la {
            n_la = n;
        }
        while n_la > 0 && la[(n_la - 1) as usize] == 0 {
            n_la -= 1;
        }
        if n_la == 0 {
            return ret_coef_one(ss);
        }
        r = 0;
        while r < n_la && mu[r as usize] < nu[r as usize] {
            r += 1;
        }
        while r < n && mu[r as usize] < nu[r as usize] && nu[r as usize] < mu[(r - n_la) as usize] {
            r += 1;
        }
        if r < n {
            let mut i_nu = r;
            let mut s = if r > n_la { r - n_la } else { 0 };
            let mut i_mu = s;
            while r < n && i_nu < n_la {
                if mu[r as usize] == nu[r as usize] {
                    mu[r as usize] = -1;
                } else {
                    nu[i_nu as usize] = nu[r as usize];
                    if nu[i_nu as usize] < la[i_nu as usize] {
                        return ss;
                    }
                    i_nu += 1;
                }
                r += 1;
            }
            while r < n {
                if mu[r as usize] == nu[r as usize] {
                    mu[r as usize] = -1;
                    r += 1;
                    continue;
                }
                while mu[s as usize] == -1 {
                    s += 1;
                }
                if mu[s as usize] < nu[r as usize] {
                    return ss;
                }
                if mu[s as usize] > nu[r as usize] {
                    mu[i_mu as usize] = mu[s as usize];
                    i_mu += 1;
                    nu[i_nu as usize] = nu[r as usize];
                    if nu[i_nu as usize] < la[i_nu as usize] {
                        return ss;
                    }
                    i_nu += 1;
                }
                r += 1;
                s += 1;
            }
            while s < n {
                if mu[s as usize] != -1 {
                    mu[i_mu as usize] = mu[s as usize];
                    i_mu += 1;
                }
                s += 1;
            }
            if i_nu < n && la[i_nu as usize] > 0 {
                return ss;
            }
            n = i_nu;
        }

        /* Remove column of size len(la) from nu/mu. */
        r = n_la;
        while r <= n && nu[(r - 1) as usize] <= mu[(r - n_la) as usize] {
            r += 1;
        }
        if r <= n {
            if r > n_la && nu[(r - 1) as usize] > mu[(r - n_la - 1) as usize] {
                return ss;
            }
            if r < n && nu[r as usize] > mu[(r - n_la) as usize] {
                return ss;
            }
            for s in (0..(r - n_la)).rev() {
                mu[s as usize] -= 1;
            }
            for s in (0..n_la).rev() {
                la[s as usize] -= 1;
            }
            for s in 0..r {
                nu[s as usize] -= 1;
                if nu[s as usize] == 0 {
                    n = s;
                    break;
                }
            }
        }
    }

    if n == 0 {
        return ret_coef_one(ss);
    }

    nu.truncate(n as usize);
    if n < n_la {
        n_la = n;
    }
    while n_la > 0 && la[(n_la - 1) as usize] == 0 {
        n_la -= 1;
    }
    la.truncate(n_la as usize);
    if n < n_mu {
        n_mu = n;
    }
    while n_mu > 0 && mu[(n_mu - 1) as usize] == 0 {
        n_mu -= 1;
    }
    mu.truncate(n_mu as usize);

    ss.outer = Some(nu.into());
    ss.inner = Some(la.into());
    ss.cont = Some(mu.into());
    ss.sign = 2;
    ss
}
