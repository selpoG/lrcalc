use super::bindings;

use anyhow::anyhow;

use super::ivector::IntVector;

pub struct LinearCombination {
    pub data: *mut bindings::ivlincomb,
    pub it: bindings::ivlc_iter,
    pub(crate) owned: bool,
}

pub struct LinearCombinationElement {
    pub data: bindings::ivlc_keyval_t,
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

impl From<*mut bindings::ivlincomb> for LinearCombination {
    fn from(from: *mut bindings::ivlincomb) -> LinearCombination {
        LinearCombination {
            data: from,
            it: bindings::ivlc_iter {
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
        unsafe {
            let ptr = bindings::ivlc_new_default();
            if ptr == std::ptr::null_mut() {
                panic!("Memory Error")
            }
            let lc: LinearCombination = ptr.into();
            for (key, val) in it {
                let mut key = IntVector::new(&key[..]);
                if bindings::ivlc_insert(lc.data, key.data, bindings::iv_hash(key.data), val)
                    == std::ptr::null_mut()
                {
                    panic!("Memory Error")
                }
                key.owned = false;
            }
            lc
        }
    }
    #[allow(dead_code)]
    pub fn iter(&self) -> LinearCombination {
        LinearCombination {
            data: self.data,
            it: bindings::ivlc_iter {
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
        unsafe {
            let kv = bindings::ivlc_lookup(self.data, key.data, bindings::iv_hash(key.data) as u32);
            if kv == std::ptr::null_mut() {
                None
            } else {
                Some(LinearCombinationElement { data: *kv })
            }
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
                    if kv.data.value != n {
                        return DiffResult::ValueMismatch(sh, n, kv.data.value);
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
        unsafe {
            if !self.it.initialized {
                bindings::ivlc_first(self.data, &mut self.it);
                self.it.initialized = true
            } else {
                bindings::ivlc_next(&mut self.it)
            }
            if bindings::ivlc_good(&mut self.it) {
                let kv = *bindings::ivlc_keyval(&self.it);
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
}

impl Drop for LinearCombination {
    fn drop(&mut self) {
        unsafe {
            if self.owned && self.data != std::ptr::null_mut() {
                bindings::ivlc_free_all(self.data)
            }
        }
    }
}
