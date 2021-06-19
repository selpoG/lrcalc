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
		iv_cmp(&self[..], &other[..])
	}
}

pub fn iv_cmp(v: &[i32], w: &[i32]) -> std::cmp::Ordering {
	match v.len().cmp(&w.len()) {
		std::cmp::Ordering::Equal => {}
		c @ _ => return c,
	}
	for i in 0..v.len() {
		match v[i].cmp(&w[i]) {
			std::cmp::Ordering::Equal => {}
			c @ _ => return c,
		}
	}
	std::cmp::Ordering::Equal
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
pub fn iv_new(length: u32) -> *mut IntVector {
	IntVector::from_vec(vec![0; length as usize])
}

pub fn iv_hash(v: &[i32]) -> u32 {
	let mut h = std::num::Wrapping(v.len() as u32);
	for x in v {
		h = ((h << 5) ^ (h >> 27)) + std::num::Wrapping(*x as u32);
	}
	h.0
}

pub fn iv_sum(v: &IntVector) -> i32 {
	(&v[..]).iter().sum()
}

pub fn iv_free(v: &mut IntVector) {
	if v.length == 0 {
		return;
	}
	let s = unsafe { std::slice::from_raw_parts_mut((*v).array, (*v).length as usize) };
	unsafe { drop(Box::from_raw(s)) }
}

pub fn iv_free_ptr(v: *mut IntVector) {
	if v == std::ptr::null_mut() {
		return;
	}
	iv_free(unsafe { &mut *v });
	unsafe { drop(Box::from_raw(v)) }
}
