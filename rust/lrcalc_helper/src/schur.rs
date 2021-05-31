use super::ivector::{iv_free, iv_hash, IntVector};
use super::ivlincomb::{
	ivlc_add_element, ivlc_card, ivlc_new, ivlc_new_default, ivlc_reset, LinearCombination,
	LinearCombinationIter, LC_COPY_KEY, LC_FREE_KEY, LC_FREE_ZERO,
};
use super::lrcoef::lrcoef_count;
use super::lriter::{lrit_expand, lrit_free, lrit_good, lrit_new, lrit_next, LRTableauIterator};
use super::optim::{optim_coef, optim_fusion, optim_mult, optim_skew, sksh_dealloc};
use super::part::{part_entry_rs, part_valid};

#[no_mangle]
pub extern "C" fn schur_mult(
	sh1: *const IntVector,
	sh2: *const IntVector,
	rows: i32,
	cols: i32,
	partsz: i32,
) -> *mut LinearCombination {
	let mut ss = optim_mult(sh1, sh2, rows, cols);
	let lc = if ss.sign != 0 {
		lrit_expand(ss.outer, std::ptr::null(), ss.cont, rows, cols, partsz)
	} else {
		ivlc_new(5, 2)
	};
	sksh_dealloc(&mut ss);
	lc
}

fn fusion_reduce(la: &mut IntVector, level: i32, tmp: &mut [i32]) -> i32 {
	debug_assert!(la.length == tmp.len() as u32);
	let rows = la.length as i32;
	let n = rows + level;

	let mut q = 0;
	for i in 0..rows {
		let a = la[i as usize] + rows - i - 1;
		let b = if a >= 0 { a / n } else { -((n - 1 - a) / n) };
		q += b;
		tmp[i as usize] = a - b * n - rows + 1;
	}

	/* bubble sort */
	let mut sign = if (rows & 1) != 0 { 0 } else { q };
	for i in 0..(rows - 1) {
		let mut k = i;
		let mut a = tmp[k as usize];
		for j in (i + 1)..rows {
			if a < tmp[j as usize] {
				k = j;
				a = tmp[k as usize];
			}
		}
		if k != i {
			tmp[k as usize] = tmp[i as usize];
			tmp[i as usize] = a;
			sign += 1;
		}
	}

	for i in 0..rows {
		if i > 0 && tmp[(i - 1) as usize] == tmp[i as usize] {
			return 0;
		}
		let k = i + q;
		let a = tmp[i as usize] + k + (k / rows) * level;
		la[((k + rows) % rows) as usize] = a;
	}

	if (sign & 1) != 0 {
		-1
	} else {
		1
	}
}

#[no_mangle]
pub extern "C" fn fusion_reduce_lc(lc: *mut LinearCombination, level: i32) -> bool {
	struct T(Vec<*mut IntVector>);
	impl Drop for T {
		fn drop(&mut self) {
			for v in &self.0[..] {
				iv_free(*v)
			}
		}
	}
	let mut parts = T(Vec::with_capacity(ivlc_card(lc) as usize));
	let mut coefs = Vec::with_capacity(ivlc_card(lc) as usize);

	for kv in LinearCombinationIter::from(lc as *const _) {
		parts.0.push(kv.key);
		coefs.push(kv.value);
	}
	ivlc_reset(lc);

	if parts.0.len() == 0 {
		return true;
	}
	let mut tmp = {
		let sh = unsafe { &*parts.0[0] };
		vec![0i32; sh.length as usize]
	};

	/* Reduce and reinsert terms. */
	while parts.0.len() != 0 {
		let sh = unsafe { &mut *(parts.0.pop().unwrap()) };
		let c = coefs.pop().unwrap();
		let sign = fusion_reduce(sh, level, &mut tmp[..]);
		ivlc_add_element(lc, sign * c, sh, iv_hash(sh), LC_FREE_KEY | LC_FREE_ZERO);
	}
	true
}

#[no_mangle]
pub extern "C" fn schur_mult_fusion(
	sh1: *const IntVector,
	sh2: *const IntVector,
	rows: i32,
	level: i32,
) -> *mut LinearCombination {
	debug_assert!(part_valid(sh1) && part_valid(sh2));
	let mut sh1 = unsafe { &*sh1 };
	let mut sh2 = unsafe { &*sh2 };
	if part_entry_rs(&sh1[..], rows) != 0 || part_entry_rs(&sh2[..], rows) != 0 {
		return ivlc_new(5, 2);
	}
	struct T {
		tmp: Option<*mut IntVector>,
		nsh1: Option<*mut IntVector>,
		nsh2: Option<*mut IntVector>,
	}
	impl Drop for T {
		fn drop(&mut self) {
			if let Some(p) = self.tmp {
				iv_free(p)
			}
			if let Some(p) = self.nsh1 {
				iv_free(p)
			}
			if let Some(p) = self.nsh2 {
				iv_free(p)
			}
		}
	}

	let mut sign = 1;
	let mut t = T {
		tmp: None,
		nsh1: None,
		nsh2: None,
	};
	if part_entry_rs(&sh1[..], 0) - part_entry_rs(&sh1[..], rows - 1) > level {
		t.tmp = Some(IntVector::from_vec(vec![0; rows as usize]));
		t.nsh1 = Some(IntVector::from_vec(vec![0; rows as usize]));
		for i in 0..rows {
			unsafe { (*t.nsh1.unwrap())[i as usize] = part_entry_rs(&sh1[..], i) };
		}
		sign = fusion_reduce(unsafe { &mut *t.nsh1.unwrap() }, level, unsafe {
			&mut (*t.tmp.unwrap())[..]
		});
		sh1 = unsafe { &*t.nsh1.unwrap() };
	}
	if sign == 0 {
		return ivlc_new(5, 2);
	}
	if part_entry_rs(&sh2[..], 0) - part_entry_rs(&sh2[..], rows - 1) > level {
		if t.tmp.is_none() {
			t.tmp = Some(IntVector::from_vec(vec![0; rows as usize]));
		}
		t.nsh2 = Some(IntVector::from_vec(vec![0; rows as usize]));
		for i in 0..rows {
			unsafe { (*t.nsh2.unwrap())[i as usize] = part_entry_rs(&sh2[..], i) };
		}
		sign *= fusion_reduce(unsafe { &mut *t.nsh2.unwrap() }, level, unsafe {
			&mut (*t.tmp.unwrap())[..]
		});
		sh2 = unsafe { &*t.nsh2.unwrap() };
	}
	if sign == 0 {
		return ivlc_new(5, 2);
	}

	let mut ss = optim_fusion(sh1, sh2, rows, level);
	let lc = if ss.sign != 0 {
		lrit_expand(ss.outer, std::ptr::null(), ss.cont, rows, -1, rows)
	} else {
		ivlc_new(5, 2)
	};
	sksh_dealloc(&mut ss);

	fusion_reduce_lc(lc, level);

	if sign < 0 {
		LinearCombinationIter::from(lc as *const _).visit(|kv| {
			kv.value = -kv.value;
		});
	}

	lc
}

#[no_mangle]
pub extern "C" fn schur_skew(
	outer: *const IntVector,
	inner: *const IntVector,
	rows: i32,
	partsz: i32,
) -> *mut LinearCombination {
	let mut ss = optim_skew(outer, inner, std::ptr::null(), rows);
	let lc = if ss.sign != 0 {
		lrit_expand(ss.outer, ss.inner, ss.cont, rows, -1, partsz)
	} else {
		ivlc_new(5, 2)
	};
	sksh_dealloc(&mut ss);
	lc
}

fn _schur_coprod_isredundant(cont: &IntVector, rows: i32, cols: i32) -> bool {
	let mut sz1 = -rows * cols;
	for i in 0..rows {
		sz1 += cont[i as usize];
	}
	let mut sz2 = 0;
	for i in rows..cont.length as i32 {
		sz2 += cont[i as usize];
	}
	if sz1 != sz2 {
		return sz1 < sz2;
	}
	for i in 0..rows {
		let df = cont[i as usize] - cols - part_entry_rs(&cont[..], rows + i);
		if df != 0 {
			return df > 0;
		}
	}
	false
}

fn _schur_coprod_count(
	lrit: &mut LRTableauIterator,
	rows: i32,
	cols: i32,
) -> *mut LinearCombination {
	let cont = lrit.cont;
	let lc = ivlc_new_default();
	while lrit_good(lrit) {
		if _schur_coprod_isredundant(unsafe { &*cont }, rows, cols) {
			lrit_next(lrit);
			continue;
		}
		ivlc_add_element(lc, 1, cont, iv_hash(cont), LC_COPY_KEY);
		lrit_next(lrit);
	}
	lc
}

fn _schur_coprod_expand(
	outer: *const IntVector,
	content: *const IntVector,
	rows: i32,
	cols: i32,
	partsz: i32,
) -> *mut LinearCombination {
	let lrit = lrit_new(outer, std::ptr::null(), content, -1, -1, partsz);
	let lc = _schur_coprod_count(unsafe { &mut *lrit }, rows, cols);
	lrit_free(lrit);
	lc
}

#[no_mangle]
pub extern "C" fn schur_coprod(
	sh: *const IntVector,
	rows: i32,
	cols: i32,
	partsz: i32,
	all: bool,
) -> *mut LinearCombination {
	let b = IntVector::from_vec(vec![cols; rows as usize]);

	if all {
		return schur_mult(sh, b, -1, -1, partsz);
	}

	let mut ss = optim_mult(sh, b, -1, -1);

	let lc = _schur_coprod_expand(ss.outer, ss.cont, rows, cols, partsz);
	sksh_dealloc(&mut ss);
	lc
}

#[no_mangle]
pub extern "C" fn schur_lrcoef(
	outer: *const IntVector,
	inner1: *const IntVector,
	inner2: *const IntVector,
) -> i64 {
	let mut ss = optim_coef(outer, inner1, inner2);
	let coef = if ss.sign <= 1 {
		ss.sign as i64
	} else {
		lrcoef_count(ss.outer, ss.inner, ss.cont)
	};
	sksh_dealloc(&mut ss);
	coef
}
