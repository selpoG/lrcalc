use super::helper::{
    ivector::iv_new,
    ivector::IntVector as _IntVector,
    part::{part_qdegree, part_qentry, part_valid},
};

use super::perm::{dimvec_valid, perm_valid, str_iscompat};
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
