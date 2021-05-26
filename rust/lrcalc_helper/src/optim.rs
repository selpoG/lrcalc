use super::ivector::{iv_free, IntVector};
use super::part::{part_length, part_valid};

#[repr(C)]
pub struct SkewShape {
	outer: *mut IntVector,
	inner: *mut IntVector,
	cont: *mut IntVector,
	sign: i32,
}

#[no_mangle]
pub extern "C" fn sksh_print(
	outer: *const IntVector,
	inner: *const IntVector,
	cont: *const IntVector,
) {
	let mut len = part_length(outer);
	let outer = unsafe { &(*outer)[..] };
	let inner = if inner == std::ptr::null() {
		None
	} else {
		Some(unsafe { &(*inner)[..] })
	};
	let mut ilen = inner.map(|v| v.len()).unwrap_or(0) as u32;
	let (cont, clen) = if cont == std::ptr::null() {
		(None, 0)
	} else {
		(Some(unsafe { &(*cont)[..] }), part_length(cont))
	};
	if len <= ilen {
		while len > 0 && inner.unwrap()[(len - 1) as usize] == outer[(len - 1) as usize] {
			len -= 1;
		}
		ilen = len;
	}
	let mut r0 = 0;
	while r0 < ilen && inner.unwrap()[r0 as usize] == outer[r0 as usize] {
		r0 += 1;
	}
	let ss_left = if len == 0 || ilen < len {
		0
	} else {
		inner.unwrap()[(len - 1) as usize]
	};
	let ss_right = if len == 0 { 0 } else { outer[0] };

	for r in 0..clen {
		println!(
			"{}{}",
			" ".repeat((ss_right - ss_left) as usize),
			"c".repeat(cont.unwrap()[r as usize] as usize)
		);
	}

	for r in r0..len {
		let innr = if r < ilen {
			inner.unwrap()[r as usize]
		} else {
			0
		};
		let outr = outer[r as usize];
		println!(
			"{}{}",
			" ".repeat(innr as usize),
			"s".repeat((outr - innr) as usize)
		);
	}
}

/// Find optimal shape for product of Schur functions.
///
/// 1) Let outer0 be outer minus all rows of size maxcols and all columns of size maxrows.
///
/// 2) Let content0 be content minus all rows of size maxcols and all columns of size maxrows.
///
/// 3) New outer should be smaller of outer0, content0.
///
/// 4) New content should be larger shape, plus removed rows and columns.
#[no_mangle]
pub extern "C" fn optim_mult(
	sh1: *const IntVector,
	sh2: *const IntVector,
	maxrows: i32,
	maxcols: i32,
) -> SkewShape {
	/* DEBUG: Check valid input. */
	debug_assert!(part_valid(sh1), "sh1 is not a partition");
	if sh2 != std::ptr::null() {
		debug_assert!(part_valid(sh2), "sh1 is not a partition");
	}

	let v1 = unsafe { &(*sh1)[..] };
	let v2: &[i32] = if sh2 == std::ptr::null() {
		&[] // this must not be accessed!
	} else {
		unsafe { &(*sh2)[..] }
	};

	/* Find length and width of shapes. */
	let len1 = part_length(sh1) as i32;
	let sh10 = if len1 != 0 { v1[0] } else { 0 };
	let len2 = if sh2 != std::ptr::null() {
		part_length(sh2) as i32
	} else {
		0
	};
	let sh20 = if len2 != 0 { v2[0] } else { 0 };

	/* Indicate empty result. */
	let mut ss = SkewShape {
		outer: std::ptr::null_mut(),
		inner: std::ptr::null_mut(),
		cont: std::ptr::null_mut(),
		sign: 0,
	};

	/* Empty result? */
	if maxrows >= 0 && (len1 > maxrows || len2 > maxrows) {
		return ss;
	}
	if maxcols >= 0 && (sh10 > maxcols || sh20 > maxcols) {
		return ss;
	}
	if maxrows >= 0 && maxcols >= 0 {
		let mut r = if len1 + len2 < maxrows {
			len2
		} else {
			maxrows - len1
		};
		while r < len2 {
			if v1[(maxrows - r - 1) as usize] + v2[r as usize] > maxcols {
				return ss;
			}
			r += 1;
		}
	}

	/* Number of full rows and columns in shapes. */
	let fc1 = if len1 == maxrows && len1 > 0 {
		v1[(len1 - 1) as usize]
	} else {
		0
	};
	let mut fr1 = 0;
	while fr1 < len1 && v1[fr1 as usize] == maxcols {
		fr1 += 1;
	}
	let fc2 = if len2 == maxrows && len2 > 0 {
		v2[(len2 - 1) as usize]
	} else {
		0
	};
	let mut fr2 = 0;
	while fr2 < len2 && v2[fr2 as usize] == maxcols {
		fr2 += 1;
	}

	/* Find # boxes after removing full rows and columns. */
	let mut sz1 = (fr1 - len1) * fc1;
	for r in fr1..len1 {
		sz1 += v1[r as usize]
	}
	let mut sz2 = (fr2 - len2) * fc2;
	for r in fr2..len2 {
		sz2 += v2[r as usize]
	}

	struct Obj<'a> {
		v: &'a [i32],
		len: i32,
		fc: i32,
		fr: i32,
	}
	let mut o1 = &Obj {
		v: v1,
		len: len1,
		fc: fc1,
		fr: fr1,
	};
	let mut o2 = &Obj {
		v: v2,
		len: len2,
		fc: fc2,
		fr: fr2,
	};
	/* sh2 should be largest partition. */
	if sz1 > sz2 {
		let o = o1;
		o1 = o2;
		o2 = o;
	}

	/* Remove full rows and columns from sh1. */
	let mut out = vec![0; (o1.len - o1.fr) as usize];
	for r in 0..out.len() {
		out[r] = o1.v[(o1.fr as usize + r) as usize] - o1.fc
	}

	/* Add full rows and columns to sh2. */
	let clen = if o1.fc + o2.fc > 0 {
		maxrows
	} else {
		o2.len + o1.fr
	};
	let mut cont = vec![0; clen as usize];
	for r in 0..o1.fr {
		cont[r as usize] = maxcols
	}
	for r in 0..o2.len {
		cont[(o1.fr + r) as usize] = o2.v[r as usize] + o1.fc
	}
	for r in (o2.len + o1.fr)..clen {
		cont[r as usize] = o1.fc
	}

	ss.outer = IntVector::from_vec(out);
	ss.cont = IntVector::from_vec(cont);
	ss.sign = 1;
	ss
}

#[no_mangle]
pub extern "C" fn sksh_dealloc(sksh: *mut SkewShape) {
	let sksh = unsafe { &*sksh };
	iv_free(sksh.outer);
	iv_free(sksh.inner);
	iv_free(sksh.cont);
}
