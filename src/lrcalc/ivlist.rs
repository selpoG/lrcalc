use lrcalc_helper::ivector::{iv_free_ptr, IntVector as _IntVector};

use super::ivector::IntVector;

pub struct VectorList(pub Vec<*mut _IntVector>);

pub struct VectorListIter<'a>(&'a VectorList, usize);

impl VectorList {
    pub fn iter(&self) -> VectorListIter {
        VectorListIter(self, 0)
    }
}

impl<'a> Iterator for VectorListIter<'a> {
    type Item = IntVector;

    fn next(&mut self) -> Option<Self::Item> {
        if self.1 >= self.0 .0.len() {
            return None;
        }
        let v = IntVector {
            data: self.0 .0[self.1],
            owned: false,
        };
        self.1 += 1;
        Some(v)
    }
}

impl Drop for VectorList {
    fn drop(&mut self) {
        for v in self.0.iter() {
            iv_free_ptr(*v)
        }
    }
}
