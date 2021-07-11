use super::{
    part::{part_qdegree, part_qentry, part_valid},
    perm::{dimvec_valid, perm_group, perm_valid, str_iscompat},
};

#[derive(Clone)]
pub struct _IntVector {
    pub length: u32,
    pub array: Vec<i32>,
}

impl std::fmt::Debug for _IntVector {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "IntVector({:?})", self.array)
    }
}

impl From<Vec<i32>> for _IntVector {
    fn from(from: Vec<i32>) -> _IntVector {
        _IntVector {
            length: from.len() as u32,
            array: from,
        }
    }
}

impl _IntVector {
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

impl std::cmp::Ord for _IntVector {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        iv_cmp(&self[..], &other[..])
    }
}
impl std::cmp::PartialOrd for _IntVector {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl std::cmp::PartialEq for _IntVector {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == std::cmp::Ordering::Equal
    }
}
impl Eq for _IntVector {}

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

impl<I: std::slice::SliceIndex<[i32]>> std::ops::Index<I> for _IntVector {
    type Output = I::Output;
    fn index(&self, index: I) -> &Self::Output {
        std::ops::Index::index(&self.array, index)
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::IndexMut<I> for _IntVector {
    fn index_mut(&mut self, index: I) -> &mut Self::Output {
        std::ops::IndexMut::index_mut(&mut self.array, index)
    }
}

/// never returns null
pub fn iv_new(length: u32) -> _IntVector {
    vec![0; length as usize].into()
}

pub fn iv_hash(v: &[i32]) -> u32 {
    let mut h = std::num::Wrapping(v.len() as u32);
    for x in v {
        h = ((h << 5) ^ (h >> 27)) + std::num::Wrapping(*x as u32);
    }
    h.0
}

pub fn iv_sum(v: &_IntVector) -> i32 {
    (&v[..]).iter().sum()
}

pub struct IntVector(pub _IntVector);

impl IntVector {
    pub fn default(len: u32) -> IntVector {
        IntVector(iv_new(len))
    }
    pub fn new(v: &[i32]) -> IntVector {
        IntVector(v.to_vec().into())
    }
    pub fn len(&self) -> usize {
        self.0.len()
    }
    #[allow(dead_code)]
    pub fn size(&self) -> i32 {
        self[..].iter().sum()
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
        if self.len() == 0 {
            0
        } else {
            self[0] as usize
        }
    }
    #[allow(dead_code)]
    pub fn cmp_as_part(&self, other: &Self) -> Option<std::cmp::Ordering> {
        let r1 = self.rows();
        let r2 = other.rows();
        let mut ans = None;
        match r1.cmp(&other.rows()) {
            std::cmp::Ordering::Equal => {}
            c => ans = Some(c),
        }
        let v1 = &self[..];
        let v2 = &other[..];
        let r = std::cmp::min(r1, r2);
        for i in (0..r).rev() {
            match v1[i].cmp(&v2[i]) {
                std::cmp::Ordering::Equal => {}
                c => {
                    if ans != None && ans != Some(c) {
                        return None;
                    }
                    ans = Some(c)
                }
            }
        }
        ans.or(Some(std::cmp::Ordering::Equal))
    }
    #[allow(dead_code)]
    pub fn leq_as_part(&self, other: &Self) -> bool {
        matches!(
            self.cmp_as_part(other),
            Some(std::cmp::Ordering::Less) | Some(std::cmp::Ordering::Equal)
        )
    }
    pub fn to_vec(&self) -> Vec<i32> {
        self[..].to_vec()
    }
    pub fn is_partition(&self) -> bool {
        part_valid(&self[..])
    }
    pub fn is_permutation(&self) -> bool {
        perm_valid(&self[..])
    }
    pub fn is_dimvec(&self) -> bool {
        dimvec_valid(&self[..])
    }
    pub fn is_compatible_str(&self, other: &IntVector) -> bool {
        str_iscompat(&self[..], &other[..])
    }
    pub fn to_partition(&self) -> Vec<i32> {
        let arr = &self[..];
        let mut n = self.len();
        while n > 0 && arr[n - 1] == 0 {
            n -= 1;
        }
        arr[0..n].to_vec()
    }
    pub fn to_pair(&self, cols: usize) -> (Vec<i32>, Vec<i32>) {
        let arr = &self[..];
        let n = self.len();
        assert_eq!(n % 2, 0);
        let rows = n / 2;
        (
            arr[..rows]
                .iter()
                .filter(|x| **x != cols as i32)
                .map(|x| *x - cols as i32)
                .collect(),
            arr[rows..].iter().filter(|x| **x != 0).cloned().collect(),
        )
    }
    pub fn to_quantum(&self, level: i32) -> (i32, Vec<i32>) {
        let p = &self.0;
        let d = part_qdegree(&p[..], level);
        let mut n = self.len();
        while n > 0 && part_qentry(&p[..], (n - 1) as i32, d, level) == 0 {
            n -= 1
        }
        (
            d,
            (0..n)
                .map(|i| part_qentry(&p[..], i as i32, d, level))
                .collect(),
        )
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::Index<I> for IntVector {
    type Output = I::Output;
    fn index(&self, index: I) -> &Self::Output {
        &self.0[index]
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::IndexMut<I> for IntVector {
    fn index_mut(&mut self, index: I) -> &mut Self::Output {
        &mut self.0[index]
    }
}

pub fn ivl_to_vec<Output, F: Fn(IntVector) -> Output>(v: Vec<_IntVector>, f: &F) -> Vec<Output> {
    let mut ans = Vec::with_capacity(v.len());
    for k in v {
        ans.push(f(IntVector(k)));
    }
    ans
}
