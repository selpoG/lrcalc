use super::bindings;

pub struct IntVector {
	pub data: *mut bindings::ivector,
	pub owned: bool,
}

impl IntVector {
	pub fn default(len: u32) -> IntVector {
		unsafe {
			let x = bindings::iv_new(len);
			if x == std::ptr::null_mut() {
				panic!("Memory Error")
			}
			IntVector {
				data: x,
				owned: true,
			}
		}
	}
	pub fn new(v: &[i32]) -> IntVector {
		let c_v = IntVector::default(v.len() as u32);
		unsafe {
			let ptr = std::slice::from_raw_parts_mut((*c_v.data).array, v.len());
			ptr.copy_from_slice(&v);
		}
		c_v
	}
	pub fn len(&self) -> usize {
		unsafe { (*self.data).length as usize }
	}
	pub fn rows(&self) -> usize {
		let mut n = self.len();
		unsafe {
			let v = std::slice::from_raw_parts((*self.data).array, n);
			while n > 0 && v[n - 1] == 0 {
				n -= 1
			}
		}
		n
	}
	pub fn cols(&self) -> usize {
		let n = self.len();
		if n == 0 {
			0
		} else {
			unsafe { *(*self.data).array as usize }
		}
	}
	#[allow(dead_code)]
	pub fn to_vec(&self) -> Vec<i32> {
		unsafe {
			let n = self.len();
			let arr = std::slice::from_raw_parts((*self.data).array, n);
			arr.iter().cloned().collect()
		}
	}
	pub fn to_partition(&self) -> Vec<i32> {
		unsafe {
			let mut n = self.len();
			let arr = std::slice::from_raw_parts((*self.data).array, n);
			while n > 0 && arr[n - 1] == 0 {
				n -= 1;
			}
			arr[0..n].iter().cloned().collect()
		}
	}
	pub fn to_pair(&self, cols: usize) -> (Vec<i32>, Vec<i32>) {
		unsafe {
			let n = self.len();
			assert_eq!(n % 2, 0);
			let rows = n / 2;
			let arr = std::slice::from_raw_parts((*self.data).array, 2 * rows);
			(
				arr[..rows]
					.iter()
					.filter(|x| **x != cols as i32)
					.map(|x| *x - cols as i32)
					.collect(),
				arr[rows..].iter().filter(|x| **x != 0).cloned().collect(),
			)
		}
	}
	pub fn to_quantum(&self, level: i32) -> (i32, Vec<i32>) {
		unsafe {
			let d = bindings::part_qdegree(self.data, level);
			let mut n = self.len();
			while n > 0 && bindings::part_qentry(self.data, (n - 1) as i32, d, level) == 0 {
				n -= 1
			}
			(
				d,
				(0..n)
					.map(|i| bindings::part_qentry(self.data, i as i32, d, level))
					.collect(),
			)
		}
	}
}

impl Drop for IntVector {
	fn drop(&mut self) {
		unsafe {
			if self.owned && self.data != std::ptr::null_mut() {
				bindings::iv_free(self.data)
			}
		}
	}
}
