#![allow(dead_code)]
use super::bindings;

use super::ivector::IntVector;

pub struct PartIter {
    p: IntVector,
    it: bindings::part_iter,
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
        unsafe {
            let mut pitr = PartIter {
                p: p,
                it: std::mem::MaybeUninit::uninit().assume_init(),
                initialized: false,
            };
            bindings::pitr_first(
                &mut pitr.it,
                pitr.p.data,
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
            pitr
        }
    }
    pub fn new_box(p: IntVector, rows: i32, cols: i32) -> PartIter {
        unsafe {
            let mut pitr = PartIter {
                p: p,
                it: std::mem::MaybeUninit::uninit().assume_init(),
                initialized: false,
            };
            bindings::pitr_box_first(&mut pitr.it, pitr.p.data, rows, cols);
            pitr
        }
    }
    pub fn new_box_sz(p: IntVector, rows: i32, cols: i32, size: i32) -> PartIter {
        unsafe {
            let mut pitr = PartIter {
                p: p,
                it: std::mem::MaybeUninit::uninit().assume_init(),
                initialized: false,
            };
            bindings::pitr_box_sz_first(&mut pitr.it, pitr.p.data, rows, cols, size);
            pitr
        }
    }
    pub fn next(&mut self) -> Option<&IntVector> {
        unsafe {
            if !self.initialized {
                self.initialized = true
            } else {
                bindings::pitr_next(&mut self.it)
            }
            if bindings::pitr_good(&self.it) {
                Some(&self.p)
            } else {
                None
            }
        }
    }
}
