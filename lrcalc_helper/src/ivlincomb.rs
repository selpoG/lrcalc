use hashbrown::{hash_map::RawEntryMut, HashMap};

use super::ivector::{iv_cmp, iv_free, iv_free_ptr, iv_hash, IntVector};

#[derive(Copy, Clone)]
pub struct LinearCombinationElement {
    pub key: *mut IntVector,
    pub value: i32,
}

pub struct IntVectorPtr {
    pub ptr: *mut IntVector,
    hash: u32,
}

impl std::fmt::Debug for IntVectorPtr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "IntVectorPtr({:?})", (unsafe { &(*self.ptr)[..] }))
    }
}

impl PartialEq for IntVectorPtr {
    fn eq(&self, other: &Self) -> bool {
        unsafe { iv_cmp(&(*self.ptr)[..], &(*other.ptr)[..]) == std::cmp::Ordering::Equal }
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

pub(crate) fn ivlc_reset(ht: &mut LinearCombination) {
    ht.map.clear()
}

pub fn ivlc_lookup<'a>(
    ht: &'a LinearCombination,
    key: &[i32],
    hash: u32,
) -> Option<(&'a IntVectorPtr, &'a i32)> {
    ht.map.raw_entry().from_hash(hash as u64, |e| {
        iv_cmp(key, unsafe { &(*e.ptr)[..] }) == std::cmp::Ordering::Equal
    })
}

pub fn ivlc_insert(ht: &mut LinearCombination, key: &[i32], value: i32) {
    let hash = iv_hash(key);
    let key = unsafe { &mut *IntVector::from_vec(key.to_vec()) };
    _ivlc_insert(ht, key, hash, value)
}

pub(crate) fn _ivlc_insert(ht: &mut LinearCombination, key: &mut IntVector, hash: u32, value: i32) {
    match ht.map.raw_entry_mut().from_hash(hash as u64, |e| {
        iv_cmp(&key[..], unsafe { &(*e.ptr)[..] }) == std::cmp::Ordering::Equal
    }) {
        RawEntryMut::Occupied(_) => panic!("Call only if key is not in table."),
        RawEntryMut::Vacant(e) => {
            e.insert_hashed_nocheck(hash as u64, IntVectorPtr { ptr: key, hash }, value);
        }
    }
}

pub fn ivlc_free_all(ht: &mut LinearCombination) {
    for kv in ht.map.iter_mut() {
        iv_free(unsafe { &mut *kv.0.ptr })
    }
}

pub(crate) fn ivlc_add_element(
    ht: &mut LinearCombination,
    c: i32,
    mut key: &mut IntVector,
    hash: u32,
    opt: i32,
) {
    if c == 0 {
        if (opt & LC_COPY_KEY) == 0 {
            iv_free_ptr(key);
        }
        return;
    }
    match ht.map.raw_entry_mut().from_hash(hash as u64, |e| {
        iv_cmp(&key[..], unsafe { &(*e.ptr)[..] }) == std::cmp::Ordering::Equal
    }) {
        RawEntryMut::Occupied(mut e) => {
            if (opt & LC_COPY_KEY) == 0 {
                iv_free_ptr(key);
            }
            *e.get_mut() += c;
            if *e.get() == 0 && (opt & LC_FREE_ZERO) != 0 {
                let (k, _) = e.remove_entry();
                iv_free_ptr(k.ptr);
            }
        }
        RawEntryMut::Vacant(e) => {
            if (opt & LC_COPY_KEY) != 0 {
                key = unsafe { &mut *IntVector::from_vec((&key[..]).to_vec()) };
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
    for kv in src.map.iter_mut() {
        ivlc_add_element(dst, c * *kv.1, unsafe { &mut *kv.0.ptr }, kv.0.hash, opt);
    }
}
