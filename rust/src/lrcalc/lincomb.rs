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
            if bindings::ivlc_good(&mut self.it) {
                let kv = *bindings::ivlc_keyval(&self.it);
                Some((
                    IntVector {
                        data: kv.key,
                        owned: false,
                    },
                    kv.value,
                ))
            } else {
                None
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
