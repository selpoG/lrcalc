use super::bindings;
use super::ivector::{iv_free, IntVector};

#[repr(C)]
pub struct IntVectorList {
	pub array: *mut *mut IntVector,
	pub allocated: u64,
	pub length: u64,
}

impl From<&bindings::ivlist> for IntVectorList {
	fn from(v: &bindings::ivlist) -> Self {
		IntVectorList {
			array: v.array as *mut *mut IntVector,
			allocated: v.allocated,
			length: v.length,
		}
	}
}

#[no_mangle]
pub extern "C" fn ivl_new(length: u64) -> *mut IntVectorList {
	let ivl = {
		if length == 0 {
			IntVectorList {
				length: 0,
				allocated: 0,
				array: std::ptr::null_mut(),
			}
		} else {
			let vec = vec![std::ptr::null_mut(); length as usize];
			let mut buf = vec.into_boxed_slice();
			let v = IntVectorList {
				length: 0,
				allocated: buf.len() as u64,
				array: buf.as_mut_ptr(),
			};
			std::mem::forget(buf);
			v
		}
	};
	let ivl = Box::into_raw(Box::new(ivl));
	ivl
}

fn _ivl_require(lst: &mut IntVectorList, size: usize) {
	if lst.allocated as usize >= size {
		return;
	}
	let new_size = 2 * size;

	let mut vec = vec![std::ptr::null_mut(); new_size];
	let s = unsafe { std::slice::from_raw_parts_mut(lst.array, lst.allocated as usize) };
	let len = lst.length as usize;
	vec[..len].copy_from_slice(&s[..len]);
	let s = s.as_mut_ptr();
	unsafe { drop(Box::from_raw(s)) }

	lst.allocated = new_size as u64;
	let mut buf = vec.into_boxed_slice();
	lst.array = buf.as_mut_ptr();
	std::mem::forget(buf);
}

#[no_mangle]
pub extern "C" fn ivl_append(lst: *mut IntVectorList, x: *mut IntVector) {
	let lst = unsafe { &mut *lst };
	_ivl_require(lst, (lst.length + 1) as usize);
	let room = unsafe { &mut *lst.array.offset(lst.length as isize) };
	*room = x;
	lst.length += 1;
}

#[no_mangle]
pub extern "C" fn ivl_poplast(lst: *mut IntVectorList) -> *mut IntVector {
	let lst = unsafe { &mut *lst };
	debug_assert!(lst.length > 0);
	lst.length -= 1;
	unsafe { *lst.array.offset(lst.length as isize) }
}

fn _ivl_free(lst: *mut IntVectorList, all: bool) {
	if lst == std::ptr::null_mut() {
		return;
	}
	if unsafe { (*lst).array } != std::ptr::null_mut() {
		let lst = unsafe { &*lst };
		let s = unsafe { std::slice::from_raw_parts_mut(lst.array, lst.allocated as usize) };
		if all {
			for v in s[..lst.length as usize].iter() {
				iv_free(*v as *mut IntVector)
			}
		}
		let s = s.as_mut_ptr();
		unsafe { drop(Box::from_raw(s)) }
	}
	unsafe { drop(Box::from_raw(lst)) }
}

#[no_mangle]
pub extern "C" fn ivl_free(lst: *mut IntVectorList) {
	_ivl_free(lst, false)
}

#[no_mangle]
pub extern "C" fn ivl_free_all(lst: *mut IntVectorList) {
	_ivl_free(lst, true)
}