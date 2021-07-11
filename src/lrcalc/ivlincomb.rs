use anyhow::anyhow;
use hashbrown::{hash_map::RawEntryMut, HashMap};

use super::ivector::{iv_cmp, iv_hash, IntVector};

pub struct LinearCombinationElement<'a> {
    pub key: &'a mut IntVector,
    pub value: i32,
}

pub struct IntVectorPtr {
    pub ptr: IntVector,
    hash: u32,
}

impl std::fmt::Debug for IntVectorPtr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "IntVectorPtr({:?})", (&self.ptr[..]))
    }
}

impl PartialEq for IntVectorPtr {
    fn eq(&self, other: &Self) -> bool {
        iv_cmp(&self.ptr[..], &other.ptr[..]) == std::cmp::Ordering::Equal
    }
}
impl Eq for IntVectorPtr {}

impl std::hash::Hash for IntVectorPtr {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        state.write_u32(self.hash)
    }
}

#[derive(Default, Debug)]
pub struct MyHasher(u32);

impl std::hash::Hasher for MyHasher {
    fn write(&mut self, bytes: &[u8]) {
        debug_assert!(bytes.len() == 4);
        self.0 = ((bytes[3] as u32) << 24)
            | ((bytes[2] as u32) << 16)
            | ((bytes[1] as u32) << 8)
            | (bytes[0] as u32);
    }

    fn finish(&self) -> u64 {
        self.0 as u64
    }
}

type MyBuildHasher = std::hash::BuildHasherDefault<MyHasher>;

pub struct LinearCombination {
    pub map: HashMap<IntVectorPtr, i32, MyBuildHasher>,
}

impl LinearCombination {
    pub fn clear(&mut self) {
        self.map.clear()
    }
}

pub const LC_COPY_KEY: i32 = 1;
pub const LC_FREE_KEY: i32 = 0;

pub const LC_FREE_ZERO: i32 = 2;

const IVLC_ARRAY_SZ: u32 = 100;

pub fn ivlc_new_default() -> LinearCombination {
    ivlc_new(IVLC_ARRAY_SZ)
}

pub fn ivlc_new(eltsz: u32) -> LinearCombination {
    let mut map = HashMap::default();
    map.reserve(eltsz as usize);
    LinearCombination { map }
}

pub fn ivlc_lookup<'a>(
    ht: &'a LinearCombination,
    key: &[i32],
    hash: u32,
) -> Option<(&'a IntVectorPtr, &'a i32)> {
    ht.map.raw_entry().from_hash(hash as u64, |e| {
        iv_cmp(key, &e.ptr[..]) == std::cmp::Ordering::Equal
    })
}

pub fn ivlc_insert(ht: &mut LinearCombination, key: &[i32], value: i32) {
    let hash = iv_hash(key);
    let key = key.to_vec().into();
    _ivlc_insert(ht, key, hash, value)
}

pub(crate) fn _ivlc_insert(ht: &mut LinearCombination, key: IntVector, hash: u32, value: i32) {
    match ht.map.raw_entry_mut().from_hash(hash as u64, |e| {
        iv_cmp(&key[..], &e.ptr[..]) == std::cmp::Ordering::Equal
    }) {
        RawEntryMut::Occupied(_) => panic!("Call only if key is not in table."),
        RawEntryMut::Vacant(e) => {
            e.insert_hashed_nocheck(hash as u64, IntVectorPtr { ptr: key, hash }, value);
        }
    }
}

pub(crate) fn ivlc_add_element(
    ht: &mut LinearCombination,
    c: i32,
    mut key: IntVector,
    hash: u32,
    opt: i32,
) {
    if c == 0 {
        return;
    }
    match ht.map.raw_entry_mut().from_hash(hash as u64, |e| {
        iv_cmp(&key[..], &e.ptr[..]) == std::cmp::Ordering::Equal
    }) {
        RawEntryMut::Occupied(mut e) => {
            *e.get_mut() += c;
            if *e.get() == 0 && (opt & LC_FREE_ZERO) != 0 {
                e.remove_entry();
            }
        }
        RawEntryMut::Vacant(e) => {
            if (opt & LC_COPY_KEY) != 0 {
                key = IntVector::from((&key[..]).to_vec());
            }
            e.insert_hashed_nocheck(hash as u64, IntVectorPtr { ptr: key, hash }, c);
        }
    }
}

pub(crate) fn ivlc_add_multiple(
    dst: &mut LinearCombination,
    c: i32,
    src: &mut LinearCombination,
    opt: i32,
) {
    for kv in src.map.drain() {
        ivlc_add_element(dst, c * kv.1, kv.0.ptr, kv.0.hash, opt);
    }
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

impl<'a> LinearCombination {
    #[allow(dead_code)]
    pub fn new<T: IntoIterator<Item = (Vec<i32>, i32)>>(it: T) -> LinearCombination {
        let mut lc = ivlc_new_default();
        for (key, val) in it {
            ivlc_insert(&mut lc, &key[..], val)
        }
        lc
    }
    pub fn map<Output, F: Fn(IntVector, i32) -> Output>(mut self, f: &F) -> Vec<Output> {
        let mut ans = Vec::with_capacity(self.map.len());
        for (k, v) in self.map.drain() {
            ans.push(f(k.ptr, v));
        }
        ans
    }
    #[allow(dead_code)]
    pub fn find(&self, key: &[i32]) -> Option<i32> {
        let kv = ivlc_lookup(self, key, iv_hash(key) as u32);
        kv.map(|v| *v.1)
    }
    #[allow(dead_code)]
    fn diff_helper<F: Fn(&IntVector, &i32) -> bool>(&self, other: &Self, filter: &F) -> DiffResult {
        for (sh, &n) in self.map.iter() {
            let sh = &sh.ptr;
            if !filter(sh, &n) {
                continue;
            }
            match other.find(&sh[..]) {
                None => {
                    return DiffResult::KeyMismatch(IntVector::new(&sh[..]), Which::Left);
                }
                Some(kv) => {
                    if kv != n {
                        return DiffResult::ValueMismatch(IntVector::new(&sh[..]), n, kv);
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
