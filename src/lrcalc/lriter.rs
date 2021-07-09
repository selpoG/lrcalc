use lrcalc_helper::{
    ivector::IntVector,
    lriter::{lrit_good, lrit_new, lrit_next, LRTableauIterator as _LRTableauIterator},
};

pub struct LRTableauIterator(_LRTableauIterator);

impl LRTableauIterator {
    pub fn new(
        outer: &IntVector,
        inner: Option<&IntVector>,
        maxrows: i32,
        maxcols: i32,
        partsz: i32,
    ) -> LRTableauIterator {
        let it = lrit_new(outer, inner, None, maxrows, maxcols, partsz);
        LRTableauIterator(it)
    }
}

impl Iterator for LRTableauIterator {
    type Item = Vec<i32>;
    fn next(&mut self) -> Option<Self::Item> {
        let it = &mut self.0;
        if lrit_good(it) {
            let len = it.size;
            let arr = &it.array[..len as usize];
            let val = arr.iter().map(|x| x.value).collect();
            lrit_next(it);
            Some(val)
        } else {
            None
        }
    }
}
