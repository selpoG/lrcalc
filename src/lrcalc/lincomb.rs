use anyhow::anyhow;

use lrcalc_helper::{
    ivector::iv_hash,
    ivlincomb::{
        ivlc_free_all, ivlc_insert, ivlc_lookup, ivlc_new_default,
        LinearCombination as _LinearCombination, LinearCombinationElement, LinearCombinationIter,
    },
};

use super::ivector::IntVector;

pub struct LinearCombination {
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
            it: LinearCombinationIter::from(from as *const _),
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
            if ivlc_insert(lc_data, &key[..], val) == std::ptr::null_mut() {
                panic!("Memory Error")
            }
        }
        lc
    }
    fn deref(&self) -> &_LinearCombination {
        unsafe { &*self.it.ht }
    }
    fn deref_mut(&mut self) -> &mut _LinearCombination {
        unsafe { &mut *(self.it.ht as *mut _) }
    }
    #[allow(dead_code)]
    pub fn iter(&self) -> LinearCombination {
        LinearCombination {
            it: LinearCombinationIter::from(self.it.ht),
            owned: false,
        }
    }
    #[allow(dead_code)]
    pub fn find(&self, key: &[i32]) -> Option<LinearCombinationElement> {
        let kv = ivlc_lookup(self.deref(), key, iv_hash(key) as u32);
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
            match other.find(&sh[..]) {
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
        self.it.next().map(|kv| {
            (
                IntVector {
                    data: kv.key,
                    owned: false,
                },
                kv.value,
            )
        })
    }
}

impl Drop for LinearCombination {
    fn drop(&mut self) {
        if self.owned && self.it.ht != std::ptr::null_mut() {
            ivlc_free_all(self.it.ht as *mut _)
        }
    }
}
