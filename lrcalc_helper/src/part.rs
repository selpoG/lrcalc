use super::ivector::IntVector;

/// General partition iterator that the compiler will optimize when opt is known at compile time.
pub struct PartitionIterator {
    part: *mut IntVector,
    outer: *const IntVector,
    inner: *const IntVector,
    length: i32,
    rows: i32,
    opt: i32,
}

impl PartitionIterator {
    fn default() -> PartitionIterator {
        PartitionIterator {
            part: std::ptr::null_mut(),
            outer: std::ptr::null(),
            inner: std::ptr::null(),
            length: 0,
            rows: -1,
            opt: 0,
        }
    }
}

pub const PITR_USE_OUTER: i32 = 1;
pub const PITR_USE_INNER: i32 = 2;
pub const PITR_USE_SIZE: i32 = 4;

pub fn part_valid(p: &[i32]) -> bool {
    let mut x = 0;
    for i in (0..p.len()).rev() {
        let y = p[i];
        if y < x {
            return false;
        }
        x = y;
    }
    true
}

pub(crate) fn part_decr(p: &[i32]) -> bool {
    for i in 1..p.len() {
        if p[i - 1] < p[i] {
            return false;
        }
    }
    true
}

pub fn part_length(v: &[i32]) -> u32 {
    let mut n = v.len();
    while n > 0 && v[n - 1] == 0 {
        n -= 1
    }
    n as u32
}

pub fn part_entry(p: &[i32], i: i32) -> i32 {
    let i = i as usize;
    if i < p.len() {
        p[i]
    } else {
        0
    }
}

pub fn part_leq(p1: &IntVector, p2: &IntVector) -> bool {
    let len = p1.length as usize;
    if len > p2.length as usize {
        return false;
    }
    for i in (0..len).rev() {
        if p1[i] > p2[i] {
            return false;
        }
    }
    true
}

// Translate fusion algebra partitions to quantum cohomology notation.

#[allow(clippy::many_single_char_names)]
pub fn part_qdegree(p: &[i32], level: i32) -> i32 {
    let n = (p.len() as i32) + level;
    let mut d = 0;
    for i in 0..p.len() as usize {
        let a = p[i] + (p.len() as i32) - (i as i32) - 1;
        let b = if a >= 0 { a / n } else { -((n - 1 - a) / n) };
        d += b;
    }
    d
}

pub fn part_qentry(p: &[i32], i: i32, d: i32, level: i32) -> i32 {
    let rows = p.len() as i32;
    let k = (i + d) % rows;
    p[k as usize] - ((i + d) / rows) * level - d
}

pub fn pitr_good(itr: &PartitionIterator) -> bool {
    itr.rows >= 0
}

#[allow(clippy::too_many_arguments)]
fn _pitr_first(
    itr: &mut PartitionIterator,
    p: &mut IntVector,
    mut rows: i32,
    cols: i32,
    outer: Option<&IntVector>,
    inner: Option<&IntVector>,
    mut size: i32,
    opt: i32,
) -> Result<(), ()> {
    let use_outer = (opt & PITR_USE_OUTER) != 0;
    let use_inner = (opt & PITR_USE_INNER) != 0;
    let use_size = (opt & PITR_USE_SIZE) != 0;

    debug_assert!(!use_outer || part_valid(&outer.unwrap()[..]));
    debug_assert!(!use_inner || part_valid(&inner.unwrap()[..]));
    debug_assert!(!use_outer || !use_inner || part_leq(inner.unwrap(), outer.unwrap()));

    itr.part = p;
    itr.outer = if use_outer {
        outer.unwrap()
    } else {
        std::ptr::null()
    };
    itr.inner = if use_inner {
        inner.unwrap()
    } else {
        std::ptr::null()
    };
    itr.opt = opt;

    let p = &mut p[..];
    let outer = outer.map(|x| &x[..]);
    let inner = inner.map(|x| &x[..]);

    if cols == 0 {
        rows = 0;
    }
    if use_size && rows > size {
        rows = size;
    }
    if use_outer {
        let outer = outer.unwrap();
        if rows as usize > outer.len() {
            rows = outer.len() as i32;
        }
        while rows > 0 && outer[(rows - 1) as usize] == 0 {
            rows -= 1;
        }
    }
    itr.rows = rows;
    itr.length = rows;
    p.fill(0);

    let mut inner_sz = 0;
    if use_inner {
        let inner = inner.unwrap();
        debug_assert!(inner.len() >= rows as usize);
        if inner.len() > rows as usize && inner[rows as usize] != 0 {
            return Err(());
        }
        if rows > 0 && cols < inner[0] {
            return Err(());
        }
    }

    if use_size {
        if size > rows * cols {
            return Err(());
        }
        if use_inner {
            inner_sz = inner.unwrap().iter().sum();
            if size < inner_sz {
                return Err(());
            }
        }
    }

    let mut r = 0;
    while r < rows {
        let mut c = cols;
        if use_outer && c > outer.unwrap()[r as usize] {
            c = outer.unwrap()[r as usize];
        }
        if use_size {
            let mut avail = size;
            if use_inner {
                inner_sz -= inner.unwrap()[r as usize];
                avail -= inner_sz;
            }
            if avail == 0 {
                itr.length = r;
                return Ok(());
            }
            if c > avail {
                c = avail;
            }
            size -= c;
        }
        p[r as usize] = c;
        r += 1
    }

    if use_size && size > 0 {
        return Err(());
    }

    itr.length = r;
    Ok(())
}

pub fn pitr_first(
    p: &mut IntVector,
    rows: i32,
    cols: i32,
    outer: Option<*const IntVector>,
    inner: Option<*const IntVector>,
    size: i32,
    opt: i32,
) -> PartitionIterator {
    let outer = outer.map(|p| unsafe { &*p });
    let inner = inner.map(|p| unsafe { &*p });
    let mut pitr = PartitionIterator::default();
    match _pitr_first(&mut pitr, p, rows, cols, outer, inner, size, opt) {
        Ok(_) => {}
        Err(_) => pitr.rows = -1,
    }
    pitr
}

pub fn pitr_box_first(p: &mut IntVector, rows: i32, cols: i32) -> PartitionIterator {
    pitr_first(p, rows, cols, None, None, 0, 0)
}

pub fn pitr_box_sz_first(p: &mut IntVector, rows: i32, cols: i32, size: i32) -> PartitionIterator {
    pitr_first(p, rows, cols, None, None, size, PITR_USE_SIZE)
}

pub fn pitr_next(itr: &mut PartitionIterator) {
    let p = unsafe { &mut (*itr.part)[..] };
    let outer = if itr.outer.is_null() {
        None
    } else {
        unsafe { Some(&(*itr.outer)[..]) }
    };
    let inner = if itr.inner.is_null() {
        None
    } else {
        unsafe { Some(&(*itr.inner)[..]) }
    };
    let rows = itr.rows;
    let opt = itr.opt;

    let use_outer = (opt & PITR_USE_OUTER) != 0;
    let use_inner = (opt & PITR_USE_INNER) != 0;
    let use_size = (opt & PITR_USE_SIZE) != 0;

    let mut outer_row = rows;
    let mut size = 0;
    let mut inner_sz = 0;
    let mut outer_sz = 0;
    if use_size {
        size = 0;
        inner_sz = 0; /* number of boxes in inner[r..]. */
        outer_sz = 0; /* number of boxes in outer[outer_row..] */
    }

    for mut r in (0..itr.length).rev() {
        if use_size {
            size += p[r as usize];
        }
        if use_size && use_inner {
            inner_sz += inner.unwrap()[r as usize];
        }

        let mut c = p[r as usize] - 1;

        if use_inner && c < inner.unwrap()[r as usize] {
            continue;
        }

        if use_size && use_outer {
            /* update outer_row and outer_sz. */
            while outer_row > 0 && outer.unwrap()[(outer_row - 1) as usize] < c {
                outer_row -= 1;
                outer_sz += outer.unwrap()[outer_row as usize];
            }
        }

        if use_size && size > c * (outer_row - r) + outer_sz {
            continue;
        }

        /* can decrease p[r]. */
        if c == 0 {
            p[r as usize] = 0;
            itr.length = r;
            return;
        }

        itr.length = rows;
        while r < outer_row {
            if !use_size && use_outer && c > outer.unwrap()[r as usize] {
                break;
            }
            if use_size {
                let mut avail = size;
                if use_inner {
                    inner_sz -= inner.unwrap()[r as usize];
                    avail -= inner_sz;
                }
                if avail == 0 {
                    break;
                }
                if c > avail {
                    c = avail;
                }
                size -= c;
            }
            p[r as usize] = c;
            r += 1
        }
        if use_outer {
            while r < rows {
                c = outer.unwrap()[r as usize];
                if use_size {
                    let mut avail = size;
                    if use_inner {
                        inner_sz -= inner.unwrap()[r as usize];
                        avail -= inner_sz;
                    }
                    if avail == 0 {
                        break;
                    }
                    if c > avail {
                        c = avail;
                    }
                    size -= c;
                }
                p[r as usize] = c;
                r += 1
            }
        }
        p[r as usize..itr.length as usize].fill(0);
        itr.length = r;
        return;
    }
    itr.rows = -1;
}
