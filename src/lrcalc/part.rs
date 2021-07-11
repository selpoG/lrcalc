#![allow(dead_code)]

use super::ivector::IntVector;

/// General partition iterator that the compiler will optimize when opt is known at compile time.
pub struct PartitionIterator<'a> {
    pub part: IntVector,
    outer: Option<&'a IntVector>,
    inner: Option<&'a IntVector>,
    length: i32,
    rows: i32,
    opt: i32,
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

pub fn part_qdegree(p: &[i32], level: i32) -> i32 {
    let n = (p.len() as i32) + level;
    let mut deg = 0;
    for i in 0..p.len() as usize {
        let a = p[i] + (p.len() as i32) - (i as i32) - 1;
        let b = if a >= 0 { a / n } else { -((n - 1 - a) / n) };
        deg += b;
    }
    deg
}

pub fn part_qentry(p: &[i32], i: i32, d: i32, level: i32) -> i32 {
    let rows = p.len() as i32;
    let k = (i + d) % rows;
    p[k as usize] - ((i + d) / rows) * level - d
}

impl<'a> PartitionIterator<'a> {
    pub fn new_box_first(p: IntVector, rows: i32, cols: i32) -> Option<PartitionIterator<'static>> {
        PartitionIterator::new(p, rows, cols, None, None, 0, 0)
    }
    pub fn new_box_sz_first(
        p: IntVector,
        rows: i32,
        cols: i32,
        size: i32,
    ) -> Option<PartitionIterator<'static>> {
        PartitionIterator::new(p, rows, cols, None, None, size, PITR_USE_SIZE)
    }
    pub fn new(
        p: IntVector,
        mut rows: i32,
        cols: i32,
        outer: Option<&'a IntVector>,
        inner: Option<&'a IntVector>,
        mut size: i32,
        opt: i32,
    ) -> Option<PartitionIterator<'a>> {
        let use_outer = (opt & PITR_USE_OUTER) != 0;
        let use_inner = (opt & PITR_USE_INNER) != 0;
        let use_size = (opt & PITR_USE_SIZE) != 0;

        debug_assert!(!use_outer || part_valid(&outer.unwrap()[..]));
        debug_assert!(!use_inner || part_valid(&inner.unwrap()[..]));
        debug_assert!(!use_outer || !use_inner || part_leq(inner.unwrap(), outer.unwrap()));

        let mut itr = PartitionIterator {
            part: p,
            outer: if use_outer {
                Some(outer.unwrap())
            } else {
                None
            },
            inner: if use_inner {
                Some(inner.unwrap())
            } else {
                None
            },
            length: 0,
            rows: -1,
            opt,
        };

        let p = &mut itr.part[..];
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
                return None;
            }
            if rows > 0 && cols < inner[0] {
                return None;
            }
        }

        if use_size {
            if size > rows * cols {
                return None;
            }
            if use_inner {
                inner_sz = inner.unwrap().iter().sum();
                if size < inner_sz {
                    return None;
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
                    return Some(itr);
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
            return None;
        }

        itr.length = r;
        Some(itr)
    }
    pub fn is_good(&self) -> bool {
        self.rows >= 0
    }
    pub fn next(&mut self) {
        let p = &mut (self.part)[..];
        let outer = self.outer;
        let inner = self.inner;
        let rows = self.rows;
        let opt = self.opt;

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

        for mut r in (0..self.length).rev() {
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
                self.length = r;
                return;
            }

            self.length = rows;
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
            p[r as usize..self.length as usize].fill(0);
            self.length = r;
            return;
        }
        self.rows = -1;
    }
}

pub struct PartIter<'a> {
    it: Option<PartitionIterator<'a>>,
    initialized: bool,
}

impl<'a> PartIter<'a> {
    pub fn new(
        p: IntVector,
        rows: i32,
        cols: i32,
        outer: Option<&'a IntVector>,
        inner: Option<&'a IntVector>,
        size: i32,
        opt: i32,
    ) -> PartIter<'a> {
        let it = PartitionIterator::new(p, rows, cols, outer, inner, size, opt);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn new_box(p: IntVector, rows: i32, cols: i32) -> PartIter<'static> {
        let it = PartitionIterator::new_box_first(p, rows, cols);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn new_box_sz(p: IntVector, rows: i32, cols: i32, size: i32) -> PartIter<'static> {
        let it = PartitionIterator::new_box_sz_first(p, rows, cols, size);
        PartIter {
            it,
            initialized: false,
        }
    }
}

impl<'a> Iterator for PartIter<'a> {
    type Item = IntVector;

    fn next(&mut self) -> Option<Self::Item> {
        match &mut self.it {
            None => None,
            Some(it) => {
                if !self.initialized {
                    self.initialized = true
                } else {
                    it.next();
                }
                if it.is_good() {
                    Some(it.part.clone())
                } else {
                    None
                }
            }
        }
    }
}
