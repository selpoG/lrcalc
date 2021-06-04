use super::ivector::{iv_free, iv_free_rs, iv_print, IntVector};

#[repr(C)]
#[derive(Copy, Clone)]
pub struct LinearCombinationElement {
	pub key: *mut IntVector,
	pub value: i32,
	pub hash: u32,
	pub next: u32,
}

#[repr(C)]
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
		LinearCombinationIter::from(self as *const _)
	}
}

#[repr(C)]
pub struct LinearCombinationIter {
	pub ht: *const LinearCombination,
	pub index: u64,
	pub i: u64,
	pub initialized: bool,
}

impl From<*const LinearCombination> for LinearCombinationIter {
	fn from(v: *const LinearCombination) -> Self {
		LinearCombinationIter {
			ht: v,
			index: 0,
			i: 0,
			initialized: false,
		}
	}
}

impl LinearCombinationIter {
	pub fn visit<F>(mut self, mut f: F)
	where
		F: FnMut(&mut LinearCombinationElement),
	{
		if !self.initialized {
			ivlc_first(unsafe { &*self.ht }, &mut self);
			self.initialized = true
		}
		while ivlc_good(&self) {
			let kv = unsafe { &mut *ivlc_keyval(&mut self) };
			f(kv);
			ivlc_next(&mut self)
		}
	}
}

impl Iterator for LinearCombinationIter {
	type Item = LinearCombinationElement;
	fn next(&mut self) -> Option<Self::Item> {
		if !self.initialized {
			ivlc_first(unsafe { &*self.ht }, self);
			self.initialized = true
		} else {
			ivlc_next(self)
		}
		if ivlc_good(self) {
			Some(ivlc_keyval_rs(self))
		} else {
			None
		}
	}
}

#[no_mangle]
pub extern "C" fn ivlc_new_default() -> *mut LinearCombination {
	ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ)
}

pub fn ivlc_new(tabsz: u32, eltsz: u32) -> *mut LinearCombination {
	let ivlc = {
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
			let mut vec = Vec::with_capacity(eltsz as usize);
			unsafe {
				vec.set_len(eltsz as usize);
			}
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
			table: table,
			elts_sz: 0,
			elts: elts,
		}
	};
	let ivlc = Box::into_raw(Box::new(ivlc));
	ivlc
}

#[no_mangle]
pub extern "C" fn ivlc_free(ht: *mut LinearCombination) {
	if ht == std::ptr::null_mut() {
		return;
	}
	if unsafe { (*ht).table_sz } != 0 {
		let ht = unsafe { &*ht };

		let s = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
		let s = s.as_mut_ptr();
		unsafe { drop(Box::from_raw(s)) }
	}
	if unsafe { (*ht).elts_sz } != 0 {
		let ht = unsafe { &*ht };
		let s = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
		let s = s.as_mut_ptr();
		unsafe { drop(Box::from_raw(s)) }
	}
	unsafe { drop(Box::from_raw(ht)) }
}

pub fn ivlc_reset(ht: &mut LinearCombination) {
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
	let mut elts = Vec::with_capacity(newsz);
	unsafe {
		elts.set_len(newsz);
	}
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
#[no_mangle]
pub extern "C" fn ivlc_lookup(
	ht: &LinearCombination,
	key: &IntVector,
	hash: u32,
) -> *mut LinearCombinationElement {
	let table = unsafe { std::slice::from_raw_parts_mut(ht.table, ht.table_sz as usize) };
	let elts = unsafe { std::slice::from_raw_parts_mut(ht.elts, ht.elts_sz as usize) };
	let index = hash % ht.table_sz;
	let mut i = table[index as usize];
	while i != 0 && key.cmp(unsafe { &*elts[i as usize].key }) != std::cmp::Ordering::Equal {
		i = elts[i as usize].next;
	}
	if i == 0 {
		std::ptr::null_mut()
	} else {
		&mut elts[i as usize]
	}
}

#[no_mangle]
pub extern "C" fn ivlc_equals(ht1: &LinearCombination, ht2: &LinearCombination) -> bool {
	for kv1 in LinearCombinationIter::from(ht1 as *const _) {
		if kv1.value == 0 {
			continue;
		}
		let kv2 = ivlc_lookup(ht2, unsafe { &*kv1.key }, kv1.hash);
		if kv2 == std::ptr::null_mut() || kv1.value != unsafe { &*kv2 }.value {
			return false;
		}
	}
	for kv2 in LinearCombinationIter::from(ht2 as *const _) {
		if kv2.value == 0 {
			continue;
		}
		let kv1 = ivlc_lookup(ht1, unsafe { &*kv2.key }, kv2.hash);
		if kv1 == std::ptr::null_mut() || unsafe { &*kv1 }.value != kv2.value {
			return false;
		}
	}
	true
}

/// Call only if key is not in table.
/// Insert key into table and return a pointer to new value variable.
#[no_mangle]
pub extern "C" fn ivlc_insert(
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

#[no_mangle]
pub extern "C" fn ivlc_free_all(ht: *mut LinearCombination) {
	for kv in LinearCombinationIter::from(ht as *const _) {
		iv_free_rs(unsafe { &mut *kv.key });
	}
	ivlc_free(ht);
}

pub fn ivlc_add_element(
	ht: &mut LinearCombination,
	c: i32,
	mut key: &mut IntVector,
	hash: u32,
	opt: i32,
) {
	if c == 0 {
		if (opt & LC_COPY_KEY) == 0 {
			iv_free(key);
		}
		return;
	}
	let kv = ivlc_lookup(ht, key, hash);
	if kv != std::ptr::null_mut() {
		let kv = unsafe { &mut *kv };
		if (opt & LC_COPY_KEY) == 0 {
			iv_free(key);
		}
		kv.value += c;
		if kv.value == 0 && (opt & LC_FREE_ZERO) != 0 {
			_ivlc_remove(ht, unsafe { &*kv.key }, hash);
			iv_free(kv.key);
		}
		return;
	}
	_ivlc_require(ht, (ht.card + 1) as usize);
	if (opt & LC_COPY_KEY) != 0 {
		key = unsafe { &mut *IntVector::from_vec((&key[..]).to_vec()) };
	}
	ivlc_insert(ht, key, hash, c);
}

pub fn ivlc_add_multiple(
	dst: &mut LinearCombination,
	c: i32,
	src: &mut LinearCombination,
	opt: i32,
) {
	for kv in LinearCombinationIter::from(src as *const _) {
		ivlc_add_element(dst, c * kv.value, unsafe { &mut *kv.key }, kv.hash, opt);
	}
}

#[no_mangle]
pub extern "C" fn ivlc_good(itr: &LinearCombinationIter) -> bool {
	itr.i != 0
}

#[no_mangle]
pub extern "C" fn ivlc_first(ht: &LinearCombination, itr: &mut LinearCombinationIter) {
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

#[no_mangle]
pub extern "C" fn ivlc_next(itr: &mut LinearCombinationIter) {
	let ht = unsafe { &(*itr.ht) };
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

#[no_mangle]
pub extern "C" fn ivlc_keyval(itr: &LinearCombinationIter) -> *mut LinearCombinationElement {
	unsafe { (*itr.ht).elts.offset(itr.i as isize) }
}

pub fn ivlc_keyval_rs(itr: &LinearCombinationIter) -> LinearCombinationElement {
	unsafe { *ivlc_keyval(itr) }
}

#[no_mangle]
pub extern "C" fn ivlc_print(ht: &LinearCombination) {
	for kv in ht.iter() {
		if kv.value == 0 {
			continue;
		}
		print!("{}  ", kv.value);
		iv_print(unsafe { &*kv.key });
		println!();
	}
}

#[no_mangle]
pub extern "C" fn ivlc_print_coprod(ht: &LinearCombination, rows: u32, cols: i32) {
	for kv in ht.iter() {
		if kv.value == 0 {
			continue;
		}
		print!("{}  (", kv.value);

		let part = &unsafe { &*kv.key }[..];
		for i in 0..rows {
			if part[i as usize] <= cols {
				break;
			}
			if i > 0 {
				print!(",");
			}
			print!("{}", part[i as usize] - cols)
		}
		print!(")  (");
		for i in rows..(part.len() as u32) {
			if part[i as usize] == 0 {
				break;
			}
			if i > rows {
				print!(",");
			}
			print!("{}", part[i as usize])
		}
		println!(")");
	}
}
