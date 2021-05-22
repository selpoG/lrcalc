use super::ivector::{iv_print, IntVector};

#[repr(C)]
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

impl Iterator for LinearCombinationIter {
	type Item = (IntVector, i32);
	fn next(&mut self) -> Option<Self::Item> {
		if !self.initialized {
			ivlc_first(self.ht, self);
			self.initialized = true
		} else {
			ivlc_next(self)
		}
		if ivlc_good(self) {
			unsafe {
				let kv = &*ivlc_keyval(self);
				Some(((*kv.key).clone(), kv.value))
			}
		} else {
			None
		}
	}
}

#[no_mangle]
pub extern "C" fn ivlc_good(itr: *const LinearCombinationIter) -> bool {
	unsafe { (*itr).i != 0 }
}

#[no_mangle]
pub extern "C" fn ivlc_first(ht: *const LinearCombination, itr: *mut LinearCombinationIter) {
	let itr = unsafe { &mut *itr };
	debug_assert!(!itr.initialized);
	itr.ht = ht;
	let mut index = 0;
	let table = unsafe { std::slice::from_raw_parts((*ht).table, (*ht).table_sz as usize) };
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
pub extern "C" fn ivlc_next(itr: *mut LinearCombinationIter) {
	let itr = unsafe { &mut *itr };
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
pub extern "C" fn ivlc_keyval(itr: *const LinearCombinationIter) -> *mut LinearCombinationElement {
	unsafe { (*(*itr).ht).elts.offset((*itr).i as isize) }
}

#[no_mangle]
pub extern "C" fn ivlc_print(ht: *const LinearCombination) {
	for (k, v) in unsafe { &*ht }.iter() {
		if v == 0 {
			continue;
		}
		print!("{}  ", v);
		iv_print(&k);
		println!();
	}
}

#[no_mangle]
pub extern "C" fn ivlc_print_coprod(ht: *const LinearCombination, rows: u32, cols: i32) {
	for (k, v) in unsafe { &*ht }.iter() {
		if v == 0 {
			continue;
		}
		print!("{}  (", v);

		let part = &k[..];
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
