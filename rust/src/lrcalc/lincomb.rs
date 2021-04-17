use super::bindings;

use super::ivector::IntVector;

pub struct LinearCombination {
    pub data: *mut bindings::ivlincomb,
    pub it: bindings::ivlc_iter,
    initialized: bool,
}

impl LinearCombination {
    pub fn new(ptr: *mut bindings::ivlincomb) -> LinearCombination {
        unsafe {
            LinearCombination {
                data: ptr,
                it: std::mem::MaybeUninit::uninit().assume_init(),
                initialized: false,
            }
        }
    }
}

impl Iterator for LinearCombination {
    type Item = (IntVector, i32);
    fn next(&mut self) -> Option<Self::Item> {
        unsafe {
            if !self.initialized {
                bindings::ivlc_first(self.data, &mut self.it);
                self.initialized = true
            } else {
                bindings::ivlc_next(&mut self.it)
            }
            if bindings::ivlc_good(&mut self.it) == 0 {
                None
            } else {
                Some((
                    IntVector {
                        data: bindings::ivlc_key(&mut self.it),
                        owned: false,
                    },
                    bindings::ivlc_value(&mut self.it),
                ))
            }
        }
    }
}

impl Drop for LinearCombination {
    fn drop(&mut self) {
        unsafe {
            if self.data != std::ptr::null_mut() {
                bindings::ivlc_free_all(self.data)
            }
        }
    }
}
