use lrcalc_helper::{
    ivector::IntVector,
    lriter::{lrit_free, lrit_good, lrit_new, lrit_next, LRTableauIterator as _LRTableauIterator},
};

pub struct LRTableauIterator {
    it: *mut _LRTableauIterator,
}

impl LRTableauIterator {
    pub fn new(
        outer: *const IntVector,
        inner: *const IntVector,
        maxrows: i32,
        maxcols: i32,
        partsz: i32,
    ) -> LRTableauIterator {
        unsafe {
            let it = lrit_new(outer, inner, std::ptr::null(), maxrows, maxcols, partsz);
            LRTableauIterator { it: it }
        }
    }
}

impl Iterator for LRTableauIterator {
    type Item = Vec<i32>;
    fn next(&mut self) -> Option<Self::Item> {
        unsafe {
            if lrit_good(self.it) {
                let len = (*self.it).size;
                let arr = std::slice::from_raw_parts((*self.it).array, len as usize);
                let val = arr.iter().map(|x| x.value).collect();
                lrit_next(self.it);
                Some(val)
            } else {
                None
            }
        }
    }
}

impl Drop for LRTableauIterator {
    fn drop(&mut self) {
        unsafe { lrit_free(self.it) }
    }
}
