use super::ivector::{iv_free, iv_hash, IntVector};
use super::ivlincomb::{ivlc_add_element, ivlc_new_default, LinearCombination, LC_COPY_KEY};
use super::part::{part_decr, part_length, part_leq, part_valid};

#[repr(C)]
pub struct LRIteratorBox {
	pub value: i32,
	max: i32,
	above: i32,
	right: i32,
}

#[repr(C)]
pub struct LRTableauIterator {
	pub cont: *mut IntVector,
	pub size: i32,
	array_len: i32,
	pub array: *mut LRIteratorBox,
}

#[no_mangle]
pub extern "C" fn lrit_new(
	outer: *const IntVector,
	inner: *const IntVector,
	content: *const IntVector,
	mut maxrows: i32,
	maxcols: i32,
	mut partsz: i32,
) -> *mut LRTableauIterator {
	debug_assert!(part_valid(outer));
	debug_assert!(inner == std::ptr::null() || part_valid(inner));
	debug_assert!(content == std::ptr::null() || part_decr(content));

	/* Empty result if inner not contained in outer. */
	if inner != std::ptr::null() && !part_leq(inner, outer) {
		let lrit = LRTableauIterator {
			cont: IntVector::from_vec(vec![0]),
			size: -1,
			array_len: 0,
			array: std::ptr::null_mut(),
		};
		return Box::into_raw(Box::new(lrit));
	}

	let len = part_length(outer);
	let outer = unsafe { &(*outer)[..] };
	let inner = if inner == std::ptr::null() {
		None
	} else {
		unsafe { Some(&(*inner)[..]) }
	};
	let mut ilen = inner.map(|v| v.len() as u32).unwrap_or(0);
	if ilen > len {
		ilen = len;
	}
	let (content, clen) = if content == std::ptr::null() {
		(None, 0)
	} else {
		(Some(unsafe { &(*content)[..] }), part_length(content))
	};
	let out0 = if len == 0 { 0 } else { outer[0] };
	debug_assert!(maxcols < 0 || ilen == 0 || inner.unwrap()[0] == 0);

	/* Find number of boxes and maximal tableau entry. */
	let mut size = 0;
	let mut maxdepth = clen as i32;
	for r in 0..len {
		let inn_r = if r < ilen {
			inner.unwrap()[r as usize]
		} else {
			0
		};
		let rowsz = outer[r as usize] - inn_r;
		size += rowsz;
		if rowsz > 0 {
			maxdepth += 1;
		}
	}
	if maxrows < 0 || maxrows > maxdepth {
		maxrows = maxdepth;
	}

	/* Find size of array. */
	let mut array_len = size + 2;
	if maxcols >= 0 {
		let clim = maxcols - out0;
		let mut c1 = 0;
		for r in (0..clen).rev() {
			let c0 = content.unwrap()[r as usize];
			if c1 < c0 && c1 < maxcols && c0 > clim {
				array_len += 1;
			}
			c1 = c0;
		}
		if c1 >= maxcols {
			array_len -= 1;
		}
	}

	/* Allocate array. */
	let mut arr: Vec<LRIteratorBox> = Vec::with_capacity(array_len as usize);
	unsafe { arr.set_len(arr.capacity()) };
	let mut lrit = LRTableauIterator {
		cont: std::ptr::null_mut(),
		size: -1,
		array_len: array_len,
		array: std::ptr::null_mut(),
	};
	// lrit.array should be arr

	/* Allocate and copy content. */
	if partsz < maxrows {
		partsz = maxrows;
	}
	let partsz_u = partsz as u32;
	let mut cont = vec![0; partsz_u as usize];

	fn to_raw(
		mut lrit: LRTableauIterator,
		arr: Vec<LRIteratorBox>,
		cont: Vec<i32>,
	) -> *mut LRTableauIterator {
		lrit.cont = IntVector::from_vec(cont);
		let mut buf = arr.into_boxed_slice();
		if buf.len() == 0 {
			lrit.array = std::ptr::NonNull::dangling().as_ptr();
		} else {
			lrit.array = buf.as_mut_ptr();
			std::mem::forget(buf);
		}
		Box::into_raw(Box::new(lrit))
	}

	if maxrows < clen as i32 {
		return to_raw(lrit, arr, cont);
	} /* empty result. */
	for r in 0..clen {
		cont[r as usize] = content.unwrap()[r as usize];
	}

	/* Check for empty result. */
	if maxcols >= 0 && clen > 0 && cont[0] > maxcols {
		return to_raw(lrit, arr, cont);
	} /* empty result. */
	if maxcols >= 0 && out0 > maxcols {
		return to_raw(lrit, arr, cont);
	} /* empty result. */

	/* Initialize box structure. */
	{
		let mut s = 0;
		let mut out1 = 0;
		let mut inn0 = if len == 0 {
			out0
		} else if len <= ilen {
			inner.unwrap()[(len - 1) as usize]
		} else {
			0
		};
		for r in (0..len).rev() {
			let out2 = out1;
			let inn1 = inn0;
			out1 = outer[r as usize];
			inn0 = if r == 0 {
				out0
			} else if r <= ilen {
				inner.unwrap()[(r - 1) as usize]
			} else {
				0
			};
			if inn1 < out1 {
				maxdepth -= 1;
			}
			for c in inn1..out1 {
				let max = if c < out2 {
					arr[(s - out2 + inn1) as usize].max - 1
				} else {
					maxrows - 1
				};
				let b = &mut arr[s as usize];
				b.right = if c + 1 < out1 { s + 1 } else { array_len - 1 };
				b.above = if c >= inn0 { s + out1 - inn0 } else { size };
				b.max = if max < maxdepth { max } else { maxdepth };
				s += 1;
			}
		}
	}
	debug_assert!(maxdepth == clen as i32);

	/* Set values of extra boxes. */
	arr.last_mut().unwrap().value = maxrows - 1;
	arr[size as usize].value = -1;
	if maxcols >= 0 {
		let clim = maxcols - out0;
		let mut c1 = 0;
		let mut s = array_len - 2;
		let mut i = out0;
		for r in (0..clen).rev() {
			let c0 = content.unwrap()[r as usize];
			if c1 < c0 && c1 < maxcols && c0 > clim {
				arr[s as usize].value = r as i32;
				while i > maxcols - c0 && i > 0 {
					i -= 1;
					arr[(size - out0 + i) as usize].above = s;
				}
				s -= 1;
			}
			c1 = c0;
		}
	}

	/* Minimal LR tableau. */
	for s in (0..size).rev() {
		let b = &arr[s as usize];
		let x = arr[b.above as usize].value + 1;
		let b = &mut arr[s as usize];
		if x > b.max {
			return to_raw(lrit, arr, cont);
		} /* empty result. */
		b.value = x;
		cont[x as usize] += 1;
	}

	lrit.size = size;
	to_raw(lrit, arr, cont)
}

#[no_mangle]
pub extern "C" fn lrit_free(lrit: *mut LRTableauIterator) {
	if lrit == std::ptr::null_mut() {
		return;
	}
	{
		let lrit = unsafe { &*lrit };
		iv_free(lrit.cont);
		if lrit.array_len > 0 {
			let s = unsafe { std::slice::from_raw_parts_mut(lrit.array, lrit.array_len as usize) };
			unsafe { drop(Box::from_raw(s)) }
		}
	}
	unsafe { drop(Box::from_raw(lrit)) }
}

#[no_mangle]
pub extern "C" fn lrit_print_skewtab(
	lrit: *const LRTableauIterator,
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
	println!();
}

#[no_mangle]
pub extern "C" fn lrit_good(lrit: *const LRTableauIterator) -> bool {
	unsafe { &*lrit }.size >= 0
}

#[no_mangle]
pub extern "C" fn lrit_next(lrit: *mut LRTableauIterator) {
	let lrit = unsafe { &mut *lrit };
	let cont = unsafe { &mut (*lrit.cont)[..] };
	let array = unsafe { std::slice::from_raw_parts_mut(lrit.array, lrit.array_len as usize) };
	let size = lrit.size;
	let mut b_ind = 0;
	while b_ind != size {
		let b = &array[b_ind as usize];
		let mut max = array[b.right as usize].value;
		let b = &mut array[b_ind as usize];
		if max > b.max {
			max = b.max;
		}
		let mut x = b.value;
		cont[x as usize] -= 1;
		x += 1;
		while x <= max && cont[x as usize] == cont[(x - 1) as usize] {
			x += 1;
		}
		if x > max {
			b_ind += 1;
			continue;
		}

		/* Refill tableau with minimal values. */
		b.value = x;
		cont[x as usize] += 1;
		while b_ind != 0 {
			b_ind -= 1;
			let b = &array[b_ind as usize];
			x = array[b.above as usize].value + 1;
			let b = &mut array[b_ind as usize];
			b.value = x;
			cont[x as usize] += 1;
		}
		return;
	}
	lrit.size = -1;
}

pub fn lrit_expand(
	outer: *const IntVector,
	inner: *const IntVector,
	content: *const IntVector,
	maxrows: i32,
	maxcols: i32,
	partsz: i32,
) -> *mut LinearCombination {
	let lrit_raw = lrit_new(outer, inner, content, maxrows, maxcols, partsz);
	let cont = unsafe { &*lrit_raw }.cont;
	let lc = ivlc_new_default();
	while lrit_good(lrit_raw) {
		ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY);
		lrit_next(lrit_raw);
	}
	lrit_free(lrit_raw);
	lc
}
