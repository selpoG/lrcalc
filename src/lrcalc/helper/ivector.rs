use super::perm::perm_group;

#[derive(Clone)]
pub struct IntVector {
    pub length: u32,
    pub array: Vec<i32>,
}

impl std::fmt::Debug for IntVector {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "IntVector({:?})", self.array)
    }
}

impl From<Vec<i32>> for IntVector {
    fn from(from: Vec<i32>) -> IntVector {
        IntVector {
            length: from.len() as u32,
            array: from,
        }
    }
}

impl IntVector {
    pub fn len(&self) -> usize {
        self.length as usize
    }
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
    pub fn rows(&self) -> usize {
        let mut n = self.len();
        let v = &self[..];
        while n > 0 && v[n - 1] == 0 {
            n -= 1
        }
        n
    }
    pub fn cols(&self) -> usize {
        if self.is_empty() {
            0
        } else {
            self[0] as usize
        }
    }
    pub fn perm_group(&self) -> i32 {
        perm_group(&self[..])
    }
}

impl std::cmp::Ord for IntVector {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        iv_cmp(&self[..], &other[..])
    }
}
impl std::cmp::PartialOrd for IntVector {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl std::cmp::PartialEq for IntVector {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == std::cmp::Ordering::Equal
    }
}
impl Eq for IntVector {}

pub fn iv_cmp(v: &[i32], w: &[i32]) -> std::cmp::Ordering {
    match v.len().cmp(&w.len()) {
        std::cmp::Ordering::Equal => {}
        c => return c,
    }
    for i in 0..v.len() {
        match v[i].cmp(&w[i]) {
            std::cmp::Ordering::Equal => {}
            c => return c,
        }
    }
    std::cmp::Ordering::Equal
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::Index<I> for IntVector {
    type Output = I::Output;
    fn index(&self, index: I) -> &Self::Output {
        std::ops::Index::index(&self.array, index)
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::IndexMut<I> for IntVector {
    fn index_mut(&mut self, index: I) -> &mut Self::Output {
        std::ops::IndexMut::index_mut(&mut self.array, index)
    }
}

/// never returns null
pub fn iv_new(length: u32) -> IntVector {
    vec![0; length as usize].into()
}

pub fn iv_hash(v: &[i32]) -> u32 {
    let mut h = std::num::Wrapping(v.len() as u32);
    for x in v {
        h = ((h << 5) ^ (h >> 27)) + std::num::Wrapping(*x as u32);
    }
    h.0
}

pub fn iv_sum(v: &IntVector) -> i32 {
    (&v[..]).iter().sum()
}
