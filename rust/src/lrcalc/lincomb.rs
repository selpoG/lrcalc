use anyhow::anyhow;

use lrcalc_helper::{
    ivector::iv_hash,
    ivlincomb::{
        ivlc_first, ivlc_free_all, ivlc_good, ivlc_insert, ivlc_keyval_rs, ivlc_lookup,
        ivlc_new_default, ivlc_next, LinearCombination as _LinearCombination,
        LinearCombinationElement, LinearCombinationIter,
    },
};

use super::ivector::IntVector;

pub struct LinearCombination {
    pub data: *mut _LinearCombination,
    pub it: LinearCombinationIter,
    pub(crate) owned: bool,
}

#[allow(dead_code)]
#[derive(Debug)]
pub(crate) enum Which {
    Left,
    Right,
}
#[allow(dead_code)]
pub(crate) enum DiffResult {
    Equals,
    KeyMismatch(IntVector, Which),
    ValueMismatch(IntVector, i32, i32),
}

impl DiffResult {
    #[allow(dead_code)]
    pub fn flip(self) -> Self {
        match self {
            DiffResult::KeyMismatch(sh, w) => DiffResult::KeyMismatch(
                sh,
                match w {
                    Which::Left => Which::Right,
                    Which::Right => Which::Left,
                },
            ),
            DiffResult::ValueMismatch(sh, n1, n2) => DiffResult::ValueMismatch(sh, n2, n1),
            _ => self,
        }
    }
    #[allow(dead_code)]
    pub fn expect_equals(self) -> anyhow::Result<()> {
        match self {
            DiffResult::Equals => Ok(()),
            _ => Err(anyhow!("{}", self)),
        }
    }
}

impl std::fmt::Display for DiffResult {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            DiffResult::KeyMismatch(sh, w) => write!(
                f,
                "Key {:?} in {:?} was not found in the other",
                sh.to_vec(),
                w
            ),
            DiffResult::ValueMismatch(sh, n1, n2) => write!(
                f,
                "Values for Key {:?} differ: {} in left but {} in right",
                sh.to_vec(),
                n1,
                n2
            ),
            _ => write!(f, "Equals"),
        }
    }
}

impl From<*mut _LinearCombination> for LinearCombination {
    fn from(from: *mut _LinearCombination) -> LinearCombination {
        LinearCombination {
            data: from,
            it: LinearCombinationIter {
                ht: std::ptr::null(),
                index: 0,
                i: 0,
                initialized: false,
            },
            owned: true,
        }
    }
}

impl LinearCombination {
    #[allow(dead_code)]
    pub fn new<T: IntoIterator<Item = (Vec<i32>, i32)>>(it: T) -> LinearCombination {
        let ptr = ivlc_new_default();
        if ptr == std::ptr::null_mut() {
            panic!("Memory Error")
        }
        let mut lc: LinearCombination = ptr.into();
        let lc_data = lc.deref_mut();
        for (key, val) in it {
            let mut key = IntVector::new(&key[..]);
            let v = unsafe { &mut *key.data };
            if ivlc_insert(lc_data, v, iv_hash(v), val) == std::ptr::null_mut() {
                panic!("Memory Error")
            }
            key.owned = false;
        }
        lc
    }
    fn deref(&self) -> &_LinearCombination {
        unsafe { &*self.data }
    }
    fn deref_mut(&mut self) -> &mut _LinearCombination {
        unsafe { &mut *self.data }
    }
    #[allow(dead_code)]
    pub fn iter(&self) -> LinearCombination {
        LinearCombination {
            data: self.data,
            it: LinearCombinationIter {
                ht: std::ptr::null(),
                index: 0,
                i: 0,
                initialized: false,
            },
            owned: false,
        }
    }
    #[allow(dead_code)]
    pub fn find(&self, key: &IntVector) -> Option<LinearCombinationElement> {
        let v = unsafe { &*key.data };
        let kv = ivlc_lookup(self.deref(), v, iv_hash(v) as u32);
        if kv == std::ptr::null_mut() {
            None
        } else {
            unsafe { Some(*kv) }
        }
    }
    #[allow(dead_code)]
    fn diff_helper<F: Fn(&IntVector, &i32) -> bool>(&self, other: &Self, filter: &F) -> DiffResult {
        for (sh, n) in self.iter() {
            if !filter(&sh, &n) {
                continue;
            }
            match other.find(&sh) {
                None => {
                    return DiffResult::KeyMismatch(sh, Which::Left);
                }
                Some(kv) => {
                    if kv.value != n {
                        return DiffResult::ValueMismatch(sh, n, kv.value);
                    }
                }
            }
        }
        DiffResult::Equals
    }
    #[allow(dead_code)]
    pub(crate) fn diff<F: Fn(&IntVector, &i32) -> bool>(
        &self,
        other: &Self,
        filter: F,
    ) -> DiffResult {
        match self.diff_helper(other, &filter) {
            DiffResult::Equals => other.diff_helper(self, &filter).flip(),
            x @ _ => x,
        }
    }
}

impl Iterator for LinearCombination {
    type Item = (IntVector, i32);
    fn next(&mut self) -> Option<Self::Item> {
        if !self.it.initialized {
            ivlc_first(unsafe { &*self.data }, &mut self.it);
            self.it.initialized = true
        } else {
            ivlc_next(&mut self.it)
        }
        if ivlc_good(&self.it) {
            let kv = ivlc_keyval_rs(&self.it);
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

impl Drop for LinearCombination {
    fn drop(&mut self) {
        if self.owned && self.data != std::ptr::null_mut() {
            ivlc_free_all(self.data)
        }
    }
}
