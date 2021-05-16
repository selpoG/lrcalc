use super::bindings;

use super::ivector::IntVector;

pub struct VectorList {
    pub data: *mut bindings::ivlist,
}

impl VectorList {
    pub fn len(&self) -> usize {
        unsafe { (*self.data).length as usize }
    }
    pub fn at(&self, i: usize) -> IntVector {
        unsafe {
            IntVector {
                data: *(*self.data).array.offset(i as isize),
                owned: false,
            }
        }
    }
}

impl Drop for VectorList {
    fn drop(&mut self) {
        unsafe {
            if self.data != std::ptr::null_mut() {
                bindings::ivl_free_all(self.data)
            }
        }
    }
}
