use super::ivector::{iv_hash, iv_new_zero, IntVector};
use super::ivlincomb::{
	ivlc_add_multiple, ivlc_free, ivlc_insert, ivlc_new_default, ivlc_reset, LinearCombination,
	LinearCombinationIter, LC_FREE_ZERO,
};
use super::perm::perm_group_rs;

#[no_mangle]
pub extern "C" fn trans(w: *const IntVector, vars: i32) -> *mut LinearCombination {
	trans_rs(unsafe { &(*w)[..] }, vars)
}

fn trans_rs(w: &[i32], vars: i32) -> *mut LinearCombination {
	let res = ivlc_new_default();
	_trans(&mut w.to_vec()[..], vars, res);
	res
}

fn _trans(w: &mut [i32], mut vars: i32, res: *mut LinearCombination) {
	ivlc_reset(res);

	let n = perm_group_rs(w);

	let mut r = n - 1;
	while r > 0 && w[(r - 1) as usize] < w[r as usize] {
		r -= 1;
	}
	if r <= 0 {
		let xx = iv_new_zero(std::cmp::max(vars as u32, 1));
		ivlc_insert(res, xx, iv_hash(xx), 1);
		return;
	}
	vars = std::cmp::max(vars, r);

	let mut s = r + 1;
	while s < n && w[(r - 1) as usize] > w[s as usize] {
		s += 1;
	}

	let wr = w[(r - 1) as usize];
	let ws = w[(s - 1) as usize];
	w[(s - 1) as usize] = wr;
	w[(r - 1) as usize] = ws;

	let tmp = trans_rs(w, vars);
	for kv in LinearCombinationIter::from(tmp as *const _) {
		let xx = unsafe { &mut *kv.key };
		xx[(r - 1) as usize] += 1;
		ivlc_insert(res, xx, iv_hash(xx), kv.value);
	}

	let mut last = 0;
	let vr = w[(r - 1) as usize];
	for i in (1..r).rev() {
		let vi = w[(i - 1) as usize];
		if last < vi && vi < vr {
			last = vi;
			w[(i - 1) as usize] = vr;
			w[(r - 1) as usize] = vi;
			_trans(w, vars, tmp);
			ivlc_add_multiple(res, 1, tmp, LC_FREE_ZERO);
			w[(i - 1) as usize] = vi;
		}
	}

	w[(s - 1) as usize] = ws;
	w[(r - 1) as usize] = wr;
	ivlc_free(tmp);
}
