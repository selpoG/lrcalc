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
        let outer = unsafe { &*outer };
        let it = lrit_new(outer, inner, std::ptr::null(), maxrows, maxcols, partsz);
        LRTableauIterator { it }
    }
}

impl Iterator for LRTableauIterator {
    type Item = Vec<i32>;
    fn next(&mut self) -> Option<Self::Item> {
        let it = unsafe { &mut *self.it };
        if lrit_good(it) {
            let len = it.size;
            let arr = unsafe { std::slice::from_raw_parts(it.array, len as usize) };
            let val = arr.iter().map(|x| x.value).collect();
            lrit_next(it);
            Some(val)
        } else {
            None
        }
    }
}

impl Drop for LRTableauIterator {
    fn drop(&mut self) {
        lrit_free(self.it)
    }
}
