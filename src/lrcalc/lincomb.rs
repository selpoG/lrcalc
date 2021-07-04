use anyhow::anyhow;
use hashbrown::hash_map::Iter;

use lrcalc_helper::{
    ivector::iv_hash,
    ivlincomb::{
        ivlc_free_all, ivlc_insert, ivlc_lookup, ivlc_new_default, IntVectorPtr,
        LinearCombination as _LinearCombination,
    },
};

use super::ivector::IntVector;

pub struct LinearCombination(pub _LinearCombination);

pub struct LinearCombinationIter<'a> {
    pub ht: &'a _LinearCombination,
    pub i: Iter<'a, IntVectorPtr, i32>,
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

impl<'a> From<_LinearCombination> for LinearCombination {
    fn from(from: _LinearCombination) -> LinearCombination {
        LinearCombination(from)
    }
}

impl<'a> LinearCombination {
    #[allow(dead_code)]
    pub fn new<T: IntoIterator<Item = (Vec<i32>, i32)>>(it: T) -> LinearCombination {
        let ptr = ivlc_new_default();
        let mut lc: LinearCombination = ptr.into();
        for (key, val) in it {
            ivlc_insert(&mut lc.0, &key[..], val)
        }
        lc
    }
    #[allow(dead_code)]
    pub fn iter(&'a self) -> LinearCombinationIter<'a> {
        LinearCombinationIter {
            ht: &self.0,
            i: self.0.map.iter(),
        }
    }
    #[allow(dead_code)]
    pub fn find(&self, key: &[i32]) -> Option<i32> {
        let kv = ivlc_lookup(&self.0, key, iv_hash(key) as u32);
        kv.map(|v| *v.1)
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
                    if kv != n {
                        return DiffResult::ValueMismatch(sh, n, kv);
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
            x => x,
        }
    }
}

impl<'a> Iterator for LinearCombinationIter<'a> {
    type Item = (IntVector, i32);
    fn next(&mut self) -> Option<Self::Item> {
        self.i.next().map(|(k, &v)| {
            (
                IntVector {
                    data: k.ptr,
                    owned: false,
                },
                v,
            )
        })
    }
}

impl<'a> Drop for LinearCombination {
    fn drop(&mut self) {
        ivlc_free_all(&mut self.0)
    }
}
