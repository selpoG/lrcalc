use super::ivector::IntVector;
use super::lincomb::LinearCombination;
use super::bindings;

fn _schur_lrcoef(outer: &IntVector, inner1: &IntVector, inner2: &IntVector) -> i64 {
	unsafe { bindings::schur_lrcoef(outer.data, inner1.data, inner2.data) }
}

fn _schur_mult(
	sh1: &IntVector,
	sh2: &IntVector,
	rows: ::std::os::raw::c_int,
	cols: ::std::os::raw::c_int,
	partsz: ::std::os::raw::c_int,
) -> LinearCombination {
	unsafe {
		let ans = bindings::schur_mult(sh1.data, sh2.data, rows, cols, partsz);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}

fn _schur_mult_fusion(
	sh1: &IntVector,
	sh2: &IntVector,
	rows: ::std::os::raw::c_int,
	level: ::std::os::raw::c_int,
) -> LinearCombination {
	unsafe {
		let ans = bindings::schur_mult_fusion(sh1.data, sh2.data, rows, level);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}

fn _schur_skew(
	outer: &IntVector,
	inner: &IntVector,
	rows: ::std::os::raw::c_int,
	partsz: ::std::os::raw::c_int,
) -> LinearCombination {
	unsafe {
		let ans = bindings::schur_skew(outer.data, inner.data, rows, partsz);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}

fn _schur_coprod(sh: &IntVector, all: bool) -> LinearCombination {
	unsafe {
		let ans = bindings::schur_coprod(sh.data, sh.rows() as i32, sh.cols() as i32, -1, all);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}

fn _trans(w: &IntVector, vars: ::std::os::raw::c_int) -> LinearCombination {
	unsafe {
		let ans = bindings::trans(w.data, vars);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}
fn _mult_schubert(
	ww1: &IntVector,
	ww2: &IntVector,
	rank: ::std::os::raw::c_int,
) -> LinearCombination {
	unsafe {
		let ans = bindings::mult_schubert(ww1.data, ww2.data, rank);
		if ans == std::ptr::null_mut() {
			panic!("Memory Error")
		}
		LinearCombination::new(ans)
	}
}

pub fn lrcoef(outer: &[i32], inner_1: &[i32], inner_2: &[i32]) -> i64 {
	_schur_lrcoef(
		&IntVector::new(outer),
		&IntVector::new(inner_1),
		&IntVector::new(inner_2),
	)
}

pub fn mult(
	sh1: &[i32],
	sh2: &[i32],
	rows: Option<i32>,
	cols: Option<i32>,
) -> Vec<(Vec<i32>, i32)> {
	_schur_mult(
		&IntVector::new(sh1),
		&IntVector::new(sh2),
		rows.unwrap_or(-1),
		cols.unwrap_or(-1),
		-1,
	)
	.map(|(k, v)| (k.to_partition(), v))
	.collect()
}

pub fn mult_fusion(sh1: &[i32], sh2: &[i32], rows: i32, level: i32) -> Vec<(Vec<i32>, i32)> {
	_schur_mult_fusion(&IntVector::new(sh1), &IntVector::new(sh2), rows, level)
		.map(|(k, v)| (k.to_partition(), v))
		.collect()
}

pub fn mult_quantum(sh1: &[i32], sh2: &[i32], rows: i32, cols: i32) -> Vec<((i32, Vec<i32>), i32)> {
	_schur_mult_fusion(&IntVector::new(sh1), &IntVector::new(sh2), rows, cols)
		.map(|(k, v)| (k.to_quantum(cols), v))
		.collect()
}

pub fn skew(outer: &[i32], inner: &[i32], rows: Option<i32>) -> Vec<(Vec<i32>, i32)> {
	_schur_skew(
		&IntVector::new(outer),
		&IntVector::new(inner),
		rows.unwrap_or(-1),
		-1,
	)
	.map(|(k, v)| (k.to_partition(), v))
	.collect()
}

pub fn coprod(sh: &[i32], all: Option<bool>) -> Vec<((Vec<i32>, Vec<i32>), i32)> {
	let sh = &IntVector::new(sh);
	_schur_coprod(sh, all.unwrap_or(false))
		.map(|(k, v)| (k.to_pair(sh.cols()), v))
		.collect()
}

pub fn schubert_poly(perm: &[i32]) -> Vec<(Vec<i32>, i32)> {
	_trans(&IntVector::new(perm), 0)
		.map(|(k, v)| (k.to_vec(), v))
		.collect()
}

pub fn schubmult(w1: &[i32], w2: &[i32], rank: Option<i32>) -> Vec<(Vec<i32>, i32)> {
	_mult_schubert(&IntVector::new(w1), &IntVector::new(w2), rank.unwrap_or(0))
		.map(|(k, v)| (k.to_vec(), v))
		.collect()
}
