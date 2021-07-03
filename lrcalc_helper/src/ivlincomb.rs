use super::ivector::{iv_cmp, iv_free, iv_free_ptr, iv_hash, IntVector};

#[derive(Copy, Clone)]
pub struct LinearCombinationElement {
    pub key: *mut IntVector,
    pub value: i32,
    pub hash: u32,
    pub next: u32,
}

pub struct LinearCombination {
    pub table: *mut u32,
    pub elts: *mut LinearCombinationElement,
    pub card: u32,
    pub free_elts: u32,
    pub elts_len: u32,
    pub elts_sz: u32,
    pub table_sz: u32,
}

/// Minimal number of table entries for each element.
pub const USE_FACTOR: u32 = 2;

pub const LC_COPY_KEY: i32 = 1;
pub const LC_FREE_KEY: i32 = 0;

pub const LC_FREE_ZERO: i32 = 2;

const IVLC_HASHTABLE_SZ: u32 = 2003;
const IVLC_ARRAY_SZ: u32 = 100;

impl LinearCombination {
    pub fn iter(&self) -> LinearCombinationIter {
        LinearCombinationIter::from(self)
    }
}

pub struct LinearCombinationIter<'a> {
    pub ht: &'a LinearCombination,
    pub index: u64,
    pub i: u64,
    pub initialized: bool,
}

impl<'a> From<&'a LinearCombination> for LinearCombinationIter<'a> {
    fn from(v: &'a LinearCombination) -> Self {
        LinearCombinationIter {
            ht: v,
            index: 0,
            i: 0,
            initialized: false,
        }
    }
}

impl<'a> LinearCombinationIter<'a> {
    pub fn visit<F>(mut self, mut f: F)
    where
        F: FnMut(&mut LinearCombinationElement),
    {
        if !self.initialized {
            ivlc_first(self.ht, &mut self);
            self.initialized = true
        }
        while ivlc_good(&self) {
            f(ivlc_keyval(&mut self));
            ivlc_next(&mut self)
        }
    }
}

impl<'a> Iterator for LinearCombinationIter<'a> {
    type Item = LinearCombinationElement;
    fn next(&mut self) -> Option<Self::Item> {
        if !self.initialized {
            ivlc_first(self.ht, self);
            self.initialized = true
        } else {
            ivlc_next(self)
        }
        if ivlc_good(self) {
            Some(*ivlc_keyval(self))
        } else {
            None
        }
    }
}

pub fn ivlc_new_default() -> LinearCombination {
    ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)
}

pub fn ivlc_new(tabsz: u32, eltsz: u32) -> LinearCombination {
    let table = if tabsz == 0 {
        std::ptr::NonNull::dangling().as_ptr()
    } else {
        let vec = vec![0u32; tabsz as usize];
        let mut buf = vec.into_boxed_slice();
        let p = buf.as_mut_ptr();
        std::mem::forget(buf);
        p
    };
    let elts = if eltsz == 0 {
        std::ptr::NonNull::dangling().as_ptr()
    } else {
        let vec = vec![
            LinearCombinationElement {
                hash: 0,
                next: 0,
                key: std::ptr::null_mut(),
                value: 0
            };
            eltsz as usize
        ];
        let mut buf = vec.into_boxed_slice();
        let p = buf.as_mut_ptr();
        std::mem::forget(buf);
        p
    };
    LinearCombination {
        card: 0,
        free_elts: 0,
        elts_len: 1,
        table_sz: tabsz,
        table,
        elts_sz: 0,
        elts,
    }
}

pub fn ivlc_free(ht: &mut LinearCombination) {
    if ht.table_sz != 0 {
        let s = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
        let s = s.as_mut_ptr();
        unsafe { drop(Box::from_raw(s)) }
    }
    if ht.elts_sz != 0 {
        let s = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
        let s = s.as_mut_ptr();
        unsafe { drop(Box::from_raw(s)) }
    }
}

pub(crate) fn ivlc_reset(ht: &mut LinearCombination) {
    unsafe {
        std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize).fill(0);
    }
    ht.card = 0;
    ht.free_elts = 0;
    ht.elts_len = 1;
}

fn _ivlc_require_table(ht: &mut LinearCombination, sz: usize) {
    let mut newsz = 2 * (USE_FACTOR as usize) * sz + 1;
    if newsz % 3 == 0 {
        newsz += 2;
    }
    if newsz % 5 == 0 {
        newsz += 6;
    }
    if newsz % 7 == 0 {
        newsz += 30;
    }
    let mut newtab = vec![0u32; newsz as usize];

    // let oldtab = ht.table;
    let oldtab = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
    let elts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
    for index in 0..ht.table_sz {
        let mut i = oldtab[index as usize];
        while i != 0 {
            let newidx = elts[i as usize].hash % (newsz as u32);
            let next = elts[i as usize].next;
            elts[i as usize].next = newtab[newidx as usize];
            newtab[newidx as usize] = i;
            i = next;
        }
    }

    let mut buf = newtab.into_boxed_slice();
    ht.table_sz = newsz as u32;
    ht.table = buf.as_mut_ptr();
    std::mem::forget(buf);
    unsafe { drop(Box::from_raw(oldtab)) }
}

fn _ivlc_require_elts(ht: &mut LinearCombination, sz: usize) {
    let newsz = 2 * sz;
    let mut elts = vec![
        LinearCombinationElement {
            hash: 0,
            next: 0,
            key: std::ptr::null_mut(),
            value: 0
        };
        newsz
    ];
    let oldelts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
    elts[..oldelts.len()].copy_from_slice(oldelts);
    unsafe { drop(Box::from_raw(oldelts)) }
    let mut buf = elts.into_boxed_slice();
    ht.elts_sz = newsz as u32;
    ht.elts = buf.as_mut_ptr();
    std::mem::forget(buf);
}

fn _ivlc_require(ht: &mut LinearCombination, sz: usize) {
    if (USE_FACTOR as usize) * sz > ht.table_sz as usize {
        _ivlc_require_table(ht, sz);
    }
    /* First entry of ht->elts not used. */
    if sz + 1 > ht.elts_sz as usize {
        _ivlc_require_elts(ht, sz + 1)
    }
}

/// Return pointer to keyval_t, nullptr if key not in table.
pub fn ivlc_lookup(
    ht: &LinearCombination,
    key: &[i32],
    hash: u32,
) -> *mut LinearCombinationElement {
    let table = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
    let elts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
    let index = hash % ht.table_sz;
    let mut i = table[index as usize];
    while i != 0 && iv_cmp(key, &unsafe { &*elts[i as usize].key }[..]) != std::cmp::Ordering::Equal
    {
        i = elts[i as usize].next;
    }
    if i == 0 {
        std::ptr::null_mut()
    } else {
        &mut elts[i as usize]
    }
}

pub fn ivlc_insert(
    ht: &mut LinearCombination,
    key: &[i32],
    value: i32,
) -> *mut LinearCombinationElement {
    let hash = iv_hash(key);
    let key = unsafe { &mut *IntVector::from_vec(key.to_vec()) };
    _ivlc_insert(ht, key, hash, value)
}

/// Call only if key is not in table.
/// Insert key into table and return a pointer to new value variable.
pub(crate) fn _ivlc_insert(
    ht: &mut LinearCombination,
    key: &mut IntVector,
    hash: u32,
    value: i32,
) -> *mut LinearCombinationElement {
    _ivlc_require(ht, (ht.card + 1) as usize);
    ht.card += 1;
    let table = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
    let elts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
    let i;
    if ht.free_elts != 0 {
        i = ht.free_elts;
        ht.free_elts = elts[i as usize].next;
    } else {
        i = ht.elts_len;
        ht.elts_len += 1;
    }
    let kvs = &mut elts[i as usize];
    kvs.key = key;
    kvs.hash = hash;
    kvs.value = value;
    let index = hash % ht.table_sz;
    kvs.next = table[index as usize];
    table[index as usize] = i;
    kvs
}

/// Remove key from hashtable; return true if removed.
fn _ivlc_remove(ht: &mut LinearCombination, key: &IntVector, hash: u32) -> bool {
    let table = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
    let elts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
    let pi = &mut table[(hash % ht.table_sz) as usize];
    let i = *pi as usize;
    if i == 0 {
        return false;
    }
    if key.cmp(unsafe { &*elts[i].key }) == std::cmp::Ordering::Equal {
        ht.card -= 1;
        *pi = elts[i].next;
        elts[i].next = ht.free_elts;
        ht.free_elts = i as u32;
        return true;
    }
    let mut ipi = i;
    let mut i = elts[ipi].next as usize;
    while i != 0 && key.cmp(unsafe { &*elts[i].key }) != std::cmp::Ordering::Equal {
        ipi = i;
        i = elts[ipi].next as usize;
    }
    if i == 0 {
        return false;
    }
    ht.card -= 1;
    elts[ipi].next = elts[i].next;
    elts[i].next = ht.free_elts;
    ht.free_elts = i as u32;
    true
}

pub fn ivlc_free_all(ht: &mut LinearCombination) {
    for kv in LinearCombinationIter::from(ht as &_) {
        iv_free(unsafe { &mut *kv.key });
    }
    ivlc_free(ht);
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
    let kv = ivlc_lookup(ht, &key[..], hash);
    if !kv.is_null() {
        let kv = unsafe { &mut *kv };
        if (opt & LC_COPY_KEY) == 0 {
            iv_free_ptr(key);
        }
        kv.value += c;
        if kv.value == 0 && (opt & LC_FREE_ZERO) != 0 {
            _ivlc_remove(ht, unsafe { &*kv.key }, hash);
            iv_free_ptr(kv.key);
        }
        return;
    }
    _ivlc_require(ht, (ht.card + 1) as usize);
    if (opt & LC_COPY_KEY) != 0 {
        key = unsafe { &mut *IntVector::from_vec((&key[..]).to_vec()) };
    }
    _ivlc_insert(ht, key, hash, c);
}

pub(crate) fn ivlc_add_multiple(
    dst: &mut LinearCombination,
    c: i32,
    src: &mut LinearCombination,
    opt: i32,
) {
    for kv in LinearCombinationIter::from(src as &_) {
        ivlc_add_element(dst, c * kv.value, unsafe { &mut *kv.key }, kv.hash, opt);
    }
}

fn ivlc_good(itr: &LinearCombinationIter) -> bool {
    itr.i != 0
}

fn ivlc_first<'a>(ht: &'a LinearCombination, itr: &mut LinearCombinationIter<'a>) {
    debug_assert!(!itr.initialized);
    itr.ht = ht;
    let mut index = 0;
    let table = unsafe { std::slice::from_raw_parts(ht.table, ht.table_sz as usize) };
    while index < table.len() && table[index as usize] == 0 {
        index += 1
    }
    if index == table.len() {
        itr.i = 0;
        return;
    }
    itr.index = index as u64;
    itr.i = table[index as usize] as u64;
    itr.initialized = true;
}

fn ivlc_next(itr: &mut LinearCombinationIter) {
    let ht = itr.ht as &LinearCombination;
    let elts = unsafe { std::slice::from_raw_parts(ht.elts, ht.elts_sz as usize) };
    if elts[itr.i as usize].next != 0 {
        itr.i = elts[itr.i as usize].next as u64;
        return;
    }
    let mut index = (itr.index + 1) as usize;
    let table = unsafe { std::slice::from_raw_parts(ht.table, ht.table_sz as usize) };
    while index < table.len() && table[index] == 0 {
        index += 1;
    }
    if index == table.len() {
        itr.i = 0;
        return;
    }
    itr.index = index as u64;
    itr.i = table[index] as u64;
}

fn ivlc_keyval<'a>(itr: &'a mut LinearCombinationIter) -> &'a mut LinearCombinationElement {
    unsafe { &mut *(*itr.ht).elts.offset(itr.i as isize) }
}
