use super::bindings;

pub struct LRTableauIterator {
    it: *mut bindings::lrtab_iter,
}

impl LRTableauIterator {
    pub fn new(
        outer: *const bindings::ivector,
        inner: *const bindings::ivector,
        maxrows: i32,
        maxcols: i32,
        partsz: i32,
    ) -> LRTableauIterator {
        unsafe {
            let it = bindings::lrit_new(outer, inner, std::ptr::null(), maxrows, maxcols, partsz);
            LRTableauIterator { it: it }
        }
    }
}

impl Iterator for LRTableauIterator {
    type Item = Vec<i32>;
    fn next(&mut self) -> Option<Self::Item> {
        unsafe {
            if bindings::lrit_good(self.it) {
                let len = (*self.it).size;
                let arr = std::slice::from_raw_parts((*self.it).array, len as usize);
                let val = arr.iter().map(|x| x.value).collect();
                bindings::lrit_next(self.it);
                Some(val)
            } else {
                None
            }
        }
    }
}

impl Drop for LRTableauIterator {
    fn drop(&mut self) {
        unsafe { bindings::lrit_free(self.it) }
    }
}
