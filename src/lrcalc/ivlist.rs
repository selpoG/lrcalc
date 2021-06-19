use lrcalc_helper::ivlist::{ivl_free_all, IntVectorList};

use super::ivector::IntVector;

pub struct VectorList {
    pub data: *mut IntVectorList,
}

impl VectorList {
    pub fn len(&self) -> usize {
        unsafe { (*self.data).length as usize }
    }
    pub fn at(&self, i: usize) -> IntVector {
        IntVector {
            data: unsafe { *(*self.data).array.offset(i as isize) },
            owned: false,
        }
    }
}

impl Drop for VectorList {
    fn drop(&mut self) {
        if self.data != std::ptr::null_mut() {
            ivl_free_all(self.data)
        }
    }
}
