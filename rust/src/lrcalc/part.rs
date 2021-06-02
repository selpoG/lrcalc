#![allow(dead_code)]

use lrcalc_helper::part::{
    pitr_box_first, pitr_box_sz_first, pitr_first_rs, pitr_good, pitr_next, PartitionIterator,
};

use super::ivector::IntVector;

pub struct PartIter {
    p: IntVector,
    it: PartitionIterator,
    initialized: bool,
}

impl PartIter {
    pub fn new(
        p: IntVector,
        rows: i32,
        cols: i32,
        outer: Option<&IntVector>,
        inner: Option<&IntVector>,
        size: i32,
        opt: i32,
    ) -> PartIter {
        let it = pitr_first_rs(
            p.data,
            rows,
            cols,
            match outer {
                Some(x) => x.data,
                None => std::ptr::null(),
            },
            match inner {
                Some(x) => x.data,
                None => std::ptr::null(),
            },
            size,
            opt,
        );
        PartIter {
            p: p,
            it: it,
            initialized: false,
        }
    }
    pub fn new_box(p: IntVector, rows: i32, cols: i32) -> PartIter {
        let it = pitr_box_first(p.data, rows, cols);
        PartIter {
            p: p,
            it: it,
            initialized: false,
        }
    }
    pub fn new_box_sz(p: IntVector, rows: i32, cols: i32, size: i32) -> PartIter {
        let it = pitr_box_sz_first(p.data, rows, cols, size);
        PartIter {
            p: p,
            it: it,
            initialized: false,
        }
    }
    pub fn next(&mut self) -> Option<&IntVector> {
        if !self.initialized {
            self.initialized = true
        } else {
            pitr_next(&mut self.it)
        }
        if pitr_good(&self.it) {
            Some(&self.p)
        } else {
            None
        }
    }
}
