use super::bindings;
use super::ivlincomb::{LinearCombination, LinearCombinationIter};
use super::part::{part_length, part_qdegree, part_qentry};

#[repr(C)]
#[derive(Clone)]
pub struct IntVector {
	pub length: u32,
	pub array: *mut i32,
}

impl IntVector {
	pub fn from_box(mut buf: Box<[i32]>) -> IntVector {
		if buf.len() == 0 {
			return IntVector {
				length: 0,
				array: std::ptr::NonNull::dangling().as_ptr(),
			};
		}
		let v = IntVector {
			length: buf.len() as u32,
			array: buf.as_mut_ptr(),
		};
		std::mem::forget(buf);
		v
	}
	pub fn from_vec(vec: Vec<i32>) -> *mut IntVector {
		let buf = vec.into_boxed_slice();
		Box::into_raw(Box::new(IntVector::from_box(buf)))
	}
	pub fn cmp(&self, other: &Self) -> std::cmp::Ordering {
		let v = &self[..];
		let other = &other[..];
		match v.len().cmp(&other.len()) {
			std::cmp::Ordering::Equal => {}
			c @ _ => return c,
		}
		for i in 0..v.len() {
			match v[i].cmp(&other[i]) {
				std::cmp::Ordering::Equal => {}
				c @ _ => return c,
			}
		}
		std::cmp::Ordering::Equal
	}
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::Index<I> for IntVector {
	type Output = I::Output;
	fn index(&self, index: I) -> &Self::Output {
		let ptr = unsafe { std::slice::from_raw_parts(self.array, self.length as usize) };
		std::ops::Index::index(ptr, index)
	}
}

impl<I: std::slice::SliceIndex<[i32]>> std::ops::IndexMut<I> for IntVector {
	fn index_mut(&mut self, index: I) -> &mut Self::Output {
		let ptr = unsafe { std::slice::from_raw_parts_mut(self.array, self.length as usize) };
		std::ops::IndexMut::index_mut(ptr, index)
	}
}

/// never returns null
#[no_mangle]
pub extern "C" fn iv_new(length: u32) -> *mut IntVector {
	IntVector::from_vec(vec![0; length as usize])
}

/// never returns null
#[no_mangle]
pub extern "C" fn iv_new_zero(length: u32) -> *mut IntVector {
	iv_new(length)
}

/// never returns null
#[no_mangle]
pub extern "C" fn iv_new_copy(v: *const IntVector) -> *mut IntVector {
	let v = unsafe { &(*v)[..] };
	IntVector::from_vec(v.to_vec())
}

#[no_mangle]
pub extern "C" fn iv_cmp(v1: *const IntVector, v2: *const IntVector) -> i32 {
	let c = unsafe { (*v1).cmp(&*v2) };
	c as i32
}

#[no_mangle]
pub extern "C" fn iv_hash(v: *const IntVector) -> u32 {
	let v = unsafe { &(*v)[..] };
	let mut h = std::num::Wrapping(v.len() as u32);
	for x in v {
		h = ((h << 5) ^ (h >> 27)) + std::num::Wrapping(*x as u32);
	}
	h.0
}

#[no_mangle]
pub extern "C" fn iv_sum(v: *const IntVector) -> i32 {
	let v = unsafe { &(*v)[..] };
	v.iter().sum()
}

#[no_mangle]
pub extern "C" fn iv_print(v: *const IntVector) {
	let v = unsafe { &(*v)[..] };
	if v.len() == 0 {
		return print!("()");
	}
	print!("({}", v[0]);
	for i in 1..v.len() {
		print!(",{}", v[i])
	}
	print!(")")
}

#[no_mangle]
pub extern "C" fn iv_printnl(v: *const IntVector) {
	iv_print(v);
	println!()
}

pub fn iv_free_rs(v: &mut IntVector) {
	if v.length == 0 {
		return;
	}
	let s = unsafe { std::slice::from_raw_parts_mut((*v).array, (*v).length as usize) };
	unsafe { drop(Box::from_raw(s)) }
}

#[no_mangle]
pub extern "C" fn iv_free(v: *mut IntVector) {
	if v == std::ptr::null_mut() {
		return;
	}
	iv_free_rs(unsafe { &mut *v });
	unsafe { drop(Box::from_raw(v)) }
}

#[no_mangle]
pub extern "C" fn puts_r(s: *const std::os::raw::c_char) {
	let slice = unsafe { std::ffi::CStr::from_ptr(s) };
	println!("{}", slice.to_str().unwrap())
}

fn _maple_print_term(c: i32, v: &IntVector, letter: &str, nz: bool) {
	print!("{:+}*{}[", c, letter);

	for i in 0..v.length as usize {
		if nz && v[i] == 0 {
			break;
		}
		if i > 0 {
			print!(",");
		}
		print!("{}", v[i]);
	}
	print!("]")
}

#[no_mangle]
pub extern "C" fn maple_print_lincomb(
	ht: *const LinearCombination,
	letter: *const std::os::raw::c_char,
	nz: bool,
) {
	let slice = unsafe { std::ffi::CStr::from_ptr(letter) };
	let letter = slice.to_str().unwrap();
	print!("0");
	for kv in LinearCombinationIter::from(ht) {
		if kv.value == 0 {
			continue;
		}
		_maple_print_term(kv.value, unsafe { &*kv.key }, letter, nz);
	}
	println!();
}

fn _maple_qprint_term(c: i32, v: &IntVector, level: i32, letter: &str) {
	let d = part_qdegree(v, level);
	print!("{:+}*q^{}*{}[", c, d, letter);

	for i in 0..(*v).length {
		let x = part_qentry(v, i as i32, d, level);
		if x == 0 {
			break;
		}
		if i > 0 {
			print!(",");
		}
		print!("{}", x);
	}
	print!("]")
}

#[no_mangle]
pub extern "C" fn maple_qprint_lincomb(
	ht: *const LinearCombination,
	level: i32,
	letter: *const std::os::raw::c_char,
) {
	let slice = unsafe { std::ffi::CStr::from_ptr(letter) };
	let letter = slice.to_str().unwrap();
	print!("0");
	for kv in LinearCombinationIter::from(ht) {
		if kv.value == 0 {
			continue;
		}
		_maple_qprint_term(kv.value, unsafe { &*kv.key }, level, letter);
	}
	println!();
}

#[no_mangle]
pub extern "C" fn lrit_print_skewtab(
	lrit: *const bindings::lrtab_iter,
	outer: *const IntVector,
	inner: *const IntVector,
) {
	let lrit = unsafe { &*lrit };
	let array = lrit.array;
	let mut size = lrit.size;
	let array = unsafe { std::slice::from_raw_parts(array, size as usize) };

	let inner = if inner == std::ptr::null() {
		None
	} else {
		unsafe { Some(&(*inner)[..]) }
	};
	let ilen = inner.map(|v| v.len()).unwrap_or(0) as u32;
	let mut len = part_length(outer);
	let outer = unsafe { &(*outer)[..] };
	if len <= ilen {
		let inner = inner.unwrap();
		while len > 0 && inner[(len - 1) as usize] == outer[(len - 1) as usize] {
			len -= 1;
		}
	}
	if len == 0 {
		return;
	}

	let col_first = if ilen < len {
		0
	} else {
		inner.unwrap()[(len - 1) as usize]
	};
	let mut r = 0;
	while r < ilen && inner.unwrap()[r as usize] == outer[r as usize] {
		r += 1;
	}
	while r < len {
		let inn_r = if r >= ilen {
			0
		} else {
			inner.unwrap()[r as usize]
		};
		let out_r = outer[r as usize];
		let row_sz = out_r - inn_r;
		size -= row_sz;
		print!("{}", " ".repeat(2 * (inn_r - col_first) as usize));
		for c in 0..row_sz {
			print!("{:2}", array[(size + c) as usize].value)
		}
		println!();
		r += 1
	}
}
