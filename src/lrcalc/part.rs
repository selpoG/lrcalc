#![allow(dead_code)]

use lrcalc_helper::part::{
    pitr_box_first, pitr_box_sz_first, pitr_first, pitr_good, pitr_next, PartitionIterator,
};

use super::ivector::IntVector;

pub struct PartIter {
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
        let it = pitr_first(
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
    pub fn new_box(p: IntVector, rows: i32, cols: i32) -> PartIter {
        let it = pitr_box_first(p.0, rows, cols);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn new_box_sz(p: IntVector, rows: i32, cols: i32, size: i32) -> PartIter {
        let it = pitr_box_sz_first(p.0, rows, cols, size);
        PartIter {
            it,
            initialized: false,
        }
    }
    pub fn next(&mut self) -> Option<IntVector> {
        if !self.initialized {
            self.initialized = true
        } else {
            pitr_next(&mut self.it)
        }
        if pitr_good(&self.it) {
            Some(IntVector(self.it.part.clone()))
        } else {
            None
        }
    }
}
