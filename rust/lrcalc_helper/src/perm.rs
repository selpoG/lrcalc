use super::ivector::IntVector;
use super::ivlist::IntVectorList;

/// check w is a permutation of {1, 2, ..., n}
#[no_mangle]
pub extern "C" fn perm_valid(w: *const IntVector) -> bool {
	let w = unsafe { &(*w)[..] };
	let n = w.len();
	let mut done = vec![false; n];
	// check each of 1, ..., n appears only once
	for &x in w {
		let a = x.abs() - 1;
		if a < 0 || a as usize >= n || done[a as usize] {
			return false;
		}
		done[a as usize] = true;
	}
	true
}

#[no_mangle]
pub extern "C" fn perm_length(w: *const IntVector) -> i32 {
	let w = unsafe { &(*w)[..] };
	let mut res = 0;
	for i in 0..w.len() {
		for j in i + 1..w.len() {
			if w[i] > w[j] {
				res += 1
			}
		}
	}
	res
}

pub fn perm_group_rs(w: &[i32]) -> i32 {
	let mut i = w.len() as i32;
	while i > 0 && w[(i - 1) as usize] == i {
		i -= 1
	}
	i
}

#[no_mangle]
pub extern "C" fn perm_group(w: *const IntVector) -> i32 {
	perm_group_rs(unsafe { &(*w)[..] })
}

#[no_mangle]
pub extern "C" fn dimvec_valid(dv: *const IntVector) -> bool {
	dimvec_valid_rs(unsafe { &(*dv)[..] })
}

pub fn dimvec_valid_rs(dv: &[i32]) -> bool {
	if dv.len() == 0 || dv[0] < 0 {
		return false;
	}
	for i in 1..dv.len() {
		if dv[i - 1] > dv[i] {
			return false;
		}
	}
	true
}

/// Return true if S_w1 * S_w2 = 0 in H^*(Fl(rank)).
#[no_mangle]
pub extern "C" fn bruhat_zero(w1: *const IntVector, w2: *const IntVector, rank: i32) -> bool {
	let n1 = perm_group(w1);
	let n2 = perm_group(w2);
	if n1 > rank || n2 > rank {
		return true;
	}
	let (w1, w2) = if n1 <= n2 { (w1, w2) } else { (w2, w1) };
	let w1 = unsafe { &(*w1)[..] };
	let w2 = unsafe { &(*w2)[..] };
	let n = std::cmp::min(n1, n2);
	for q in 1..n {
		let q2 = rank - q;
		let mut r1 = 0;
		let mut r2 = 0;
		for p in 0..n - 1 {
			if w1[p as usize] <= q {
				r1 += 1;
			}
			if w2[p as usize] > q2 {
				r2 += 1;
			}
			if r1 < r2 {
				return true;
			}
		}
	}
	return false;
}

#[no_mangle]
pub extern "C" fn all_strings(dimvec: *const IntVector) -> *mut IntVectorList {
	all_strings_rs(unsafe { &(*dimvec)[..] })
}

pub fn all_strings_rs(dimvec: &[i32]) -> *mut IntVectorList {
	debug_assert!(dimvec_valid_rs(dimvec));

	let ld = dimvec.len();
	let mut cntvec = vec![0; ld];
	let n = dimvec[ld - 1];
	if n < 0 {
		return std::ptr::null_mut();
	}
	let n = n as u32;

	let mut str = vec![0; n as usize];
	{
		let mut j = 0;
		for i in 0..ld {
			while j < dimvec[i] {
				str[j as usize] = i as i32;
				j += 1;
			}
		}
	}
	let mut res: Vec<*mut IntVector> = Vec::with_capacity(200);

	if n == 0 {
		res.push(IntVector::from_vec(str));
		unsafe { res.set_len(res.capacity()) };
		let mut buf = res.into_boxed_slice();
		let ivl = IntVectorList {
			length: 1,
			allocated: buf.len() as u64,
			array: buf.as_mut_ptr(),
		};
		std::mem::forget(buf);
		return Box::into_raw(Box::new(ivl));
	}

	loop {
		res.push(IntVector::from_vec(str.clone()));
		let mut j = n - 1;
		cntvec[str[j as usize] as usize] += 1;
		while j > 0 && str[(j - 1) as usize] >= str[j as usize] {
			j -= 1;
			cntvec[str[j as usize] as usize] += 1;
		}
		if j == 0 {
			break;
		}

		let mut a = str[(j - 1) as usize];
		cntvec[a as usize] += 1;
		a += 1;
		while cntvec[a as usize] == 0 {
			a += 1;
		}
		str[(j - 1) as usize] = a;
		cntvec[a as usize] -= 1;

		for i in 0..ld {
			for _ in 0..cntvec[i] {
				str[j as usize] = i as i32;
				j += 1;
			}
			cntvec[i] = 0;
		}
	}

	let old_len = res.len();
	unsafe { res.set_len(res.capacity()) };
	let mut buf = res.into_boxed_slice();
	let ivl = IntVectorList {
		length: old_len as u64,
		allocated: buf.len() as u64,
		array: buf.as_mut_ptr(),
	};
	std::mem::forget(buf);
	Box::into_raw(Box::new(ivl))
}

#[no_mangle]
pub extern "C" fn all_perms(n: i32) -> *mut IntVectorList {
	debug_assert!(n >= 0);
	let dimvec: Vec<_> = (0..=n).collect();
	all_strings_rs(&dimvec[..])
}

#[no_mangle]
pub extern "C" fn string2perm(str: *const IntVector) -> *mut IntVector {
	let str = unsafe { &(*str)[..] };

	let n = (str.iter().max().unwrap_or(&0) + 1) as usize;

	let mut dimvec = vec![0; n];
	for &i in str.iter() {
		dimvec[i as usize] += 1;
	}
	for i in 1..n {
		dimvec[i] += dimvec[i - 1];
	}

	let mut perm = vec![0; str.len()];

	for i in (0..str.len()).rev() {
		let j = str[i] as usize;
		dimvec[j] -= 1;
		perm[dimvec[j]] = (i + 1) as i32;
	}

	IntVector::from_vec(perm)
}

#[no_mangle]
pub extern "C" fn str_iscompat(str1: *const IntVector, str2: *const IntVector) -> bool {
	str_iscompat_rs(unsafe { &(*str1)[..] }, unsafe { &(*str2)[..] })
}

pub fn str_iscompat_rs(s1: &[i32], s2: &[i32]) -> bool {
	if s1.len() != s2.len() {
		return false;
	}
	if s1.len() == 0 {
		return true;
	}
	let n = *s1.iter().max().unwrap();
	if n != *s2.iter().max().unwrap() {
		return false;
	}
	let mut cnt = vec![0; (n + 1) as usize];
	for &a in s1 {
		if a < 0 {
			return false;
		}
		cnt[a as usize] += 1
	}
	for &a in s2 {
		if a < 0 {
			return false;
		}
		cnt[a as usize] -= 1
	}
	cnt.iter().all(|&c| c == 0)
}

#[no_mangle]
pub extern "C" fn str2dimvec(str: *const IntVector) -> *mut IntVector {
	let str = unsafe { &(*str)[..] };
	let mut n = 0;
	for &i in str.iter() {
		if i < 0 {
			return std::ptr::null_mut();
		}
		if n <= i {
			n = i + 1;
		}
	}
	let mut res = vec![0; n as usize];
	for i in 0..str.len() {
		res[str[i] as usize] += 1
	}
	for i in 1..n {
		res[i as usize] += res[(i - 1) as usize];
	}
	IntVector::from_vec(res)
}

#[no_mangle]
pub extern "C" fn perm2string(perm: *const IntVector, dimvec: *const IntVector) -> *mut IntVector {
	let perm = unsafe { &(*perm)[..] };
	let dimvec = unsafe { &(*dimvec)[..] };
	let n = if dimvec.len() > 0 {
		dimvec[dimvec.len() - 1]
	} else {
		0
	};
	let mut res = vec![0; n as usize];
	let mut j: usize = 0;
	for i in 0..dimvec.len() {
		while (j as i32) < dimvec[i] {
			let wj = if j < perm.len() {
				perm[j] as usize
			} else {
				j + 1
			};
			res[wj - 1] = i as i32;
			j += 1;
		}
	}
	IntVector::from_vec(res)
}