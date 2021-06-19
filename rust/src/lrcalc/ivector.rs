use lrcalc_helper::{
    ivector::IntVector as _IntVector,
    ivector::{iv_free, iv_new},
    part::{part_qdegree, part_qentry, part_valid_rs},
    perm::{dimvec_valid_rs, perm_group_rs, perm_valid_rs, str_iscompat_rs},
};

pub struct IntVector {
    pub data: *mut _IntVector,
    pub owned: bool,
}

impl IntVector {
    pub fn default(len: u32) -> IntVector {
        let x = iv_new(len);
        IntVector {
            data: x,
            owned: true,
        }
    }
    pub fn new(v: &[i32]) -> IntVector {
        let mut c_v = IntVector::default(v.len() as u32);
        c_v[..].copy_from_slice(&v);
        c_v
    }
    fn deref(&self) -> &_IntVector {
        unsafe { &*self.data }
    }
    fn deref_mut(&mut self) -> &mut _IntVector {
        unsafe { &mut *self.data }
    }
    pub fn len(&self) -> usize {
        self.deref().length as usize
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
            c @ _ => ans = Some(c),
        }
        let v1 = &self[..];
        let v2 = &other[..];
        let r = std::cmp::min(r1, r2);
        for i in (0..r).rev() {
            match v1[i].cmp(&v2[i]) {
                std::cmp::Ordering::Equal => {}
                c @ _ => {
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
        match self.cmp_as_part(&other) {
            Some(std::cmp::Ordering::Less) | Some(std::cmp::Ordering::Equal) => true,
            _ => false,
        }
    }
    #[allow(dead_code)]
    pub fn perm_group(&self) -> i32 {
        perm_group_rs(&self[..])
    }
    pub fn to_vec(&self) -> Vec<i32> {
        self[..].iter().cloned().collect()
    }
    pub fn is_partition(&self) -> bool {
        part_valid_rs(&self[..])
    }
    pub fn is_permutation(&self) -> bool {
        perm_valid_rs(&self[..])
    }
    pub fn is_dimvec(&self) -> bool {
        dimvec_valid_rs(&self[..])
    }
    pub fn is_compatible_str(&self, other: &IntVector) -> bool {
        str_iscompat_rs(&self[..], &other[..])
    }
    pub fn to_partition(&self) -> Vec<i32> {
        let arr = &self[..];
        let mut n = self.len();
        while n > 0 && arr[n - 1] == 0 {
            n -= 1;
        }
        arr[0..n].iter().cloned().collect()
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
        let p = self.deref();
        let d = part_qdegree(&p[..], level);
        let mut n = self.len();
        while n > 0 && part_qentry(&p[..], (n - 1) as i32, d, level) == 0 {
            n -= 1
        }
        (
            d,
            (0..n).map(|i| part_qentry(&p[..], i as i32, d, level)).collect(),
        )
    }
}

impl Drop for IntVector {
    fn drop(&mut self) {
        if self.owned && self.data != std::ptr::null_mut() {
            iv_free(self.data)
        }
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::Index<I> for IntVector {
    type Output = I::Output;
    fn index(&self, index: I) -> &Self::Output {
        &self.deref()[index]
    }
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::IndexMut<I> for IntVector {
    fn index_mut(&mut self, index: I) -> &mut Self::Output {
        &mut self.deref_mut()[index]
    }
}
