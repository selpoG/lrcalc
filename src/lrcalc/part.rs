#![allow(dead_code)]

use lrcalc_helper::part::PartitionIterator;

use super::ivector::IntVector;

pub struct PartIter<'a> {
    it: Option<PartitionIterator<'a>>,
    initialized: bool,
}

impl<'a> PartIter<'a> {
    pub fn new(
        p: IntVector,
        rows: i32,
        cols: i32,
        outer: Option<&'a IntVector>,
        inner: Option<&'a IntVector>,
        size: i32,
        opt: i32,
    ) -> PartIter<'a> {
        let it = PartitionIterator::new(
            p.0,
            rows,
            cols,
            outer.map(|x| &x.0),
            inner.map(|x| &x.0),
            size,
            opt,
        );
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn new_box(p: IntVector, rows: i32, cols: i32) -> PartIter<'static> {
        let it = PartitionIterator::new_box_first(p.0, rows, cols);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn new_box_sz(p: IntVector, rows: i32, cols: i32, size: i32) -> PartIter<'static> {
        let it = PartitionIterator::new_box_sz_first(p.0, rows, cols, size);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn next(&mut self) -> Option<IntVector> {
        match &mut self.it {
            None => None,
            Some(it) => {
                if !self.initialized {
                    self.initialized = true
                } else {
                    it.next();
                }
                if it.is_good() {
                    Some(IntVector(it.part.clone()))
                } else {
                    None
                }
            }
        }
    }
}
