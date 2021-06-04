use super::ivector::{iv_sum, IntVector};
use super::part::{part_entry_rs, part_length, part_leq, part_valid};

struct LRCoefBox {
	/// integer in box of skew tableau
	value: i32,
	/// upper bound for integer in box
	max: i32,
	/// index of box above
	north: i32,
	/// index of box to the right
	east: i32,
	/// number of available integers larger than value
	se_supply: i32,
	/// number of boxes to the right and strictly below
	se_sz: i32,
	/// number of boxes strictly to the left in same row
	west_sz: i32,
	/// make size a power of 2, improves speed in x86_64
	_padding: i32,
}

struct LRCoefContent {
	/// number of boxes containing a given integer
	cont: i32,
	/// total supply of given integer
	supply: i32,
}

fn lrcoef_new_content(mu: &IntVector) -> Vec<LRCoefContent> {
	debug_assert!(part_valid(mu));
	debug_assert!(part_length(mu) > 0);

	let n = part_length(mu) as usize;
	let mu = &mu[..];
	let mut res = Vec::with_capacity(n + 1);
	unsafe {
		res.set_len(res.capacity());
	}
	res[0] = LRCoefContent {
		cont: mu[0],
		supply: mu[0],
	};
	for i in 0..n {
		res[i + 1] = LRCoefContent {
			cont: 0,
			supply: mu[i],
		}
	}
	res
}

fn lrcoef_new_skewtab(nu: &IntVector, la: &IntVector, max_value: i32) -> Vec<LRCoefBox> {
	debug_assert!(part_valid(nu));
	debug_assert!(part_valid(la));
	debug_assert!(part_leq(la, nu));
	debug_assert!(part_entry_rs(&nu[..], 0) > 0);

	let n = (iv_sum(nu) - iv_sum(la)) as usize;
	let nu = &nu[..];
	let la = &la[..];
	let mut array: Vec<LRCoefBox> = Vec::with_capacity(n + 2);
	unsafe {
		array.set_len(array.capacity());
	}

	let mut pos = n as i32;
	for r in (0..nu.len()).rev() {
		let nu_0 = if r == 0 { nu[0] } else { nu[r - 1] };
		let la_0 = if r == 0 {
			nu[0]
		} else {
			part_entry_rs(la, r as i32 - 1)
		};
		let nu_r = nu[r];
		let la_r = part_entry_rs(la, r as i32);
		let nu_1 = part_entry_rs(nu, r as i32 + 1);
		for c in la_r..nu_r {
			pos -= 1;
			let mut b = unsafe { &mut *array.as_mut_ptr().offset(pos as isize) };
			b.north = if la_0 <= c && c < nu_0 {
				pos - nu_r + la_0
			} else {
				n as i32
			};
			b.east = if c + 1 < nu_r { pos - 1 } else { n as i32 + 1 };
			b.west_sz = c - la_r;
			if c >= nu_1 {
				b.max = max_value;
				b.se_sz = 0;
			} else {
				let below = pos + nu_1 - la_r;
				b.max = array[below as usize].max - 1;
				b.se_sz = array[below as usize].se_sz + nu_1 - c;
			}
		}
	}
	array[n].value = 0;
	array[n + 1].value = max_value;
	array[n + 1].se_supply = 0;
	array
}

/// This is a low level function called from schur_lrcoef().
pub fn lrcoef_count(outer: &IntVector, inner: &IntVector, content: &IntVector) -> i64 {
	debug_assert!(iv_sum(outer) == iv_sum(inner) + iv_sum(content));
	debug_assert!(iv_sum(content) > 1);

	#[allow(non_snake_case)]
	let mut T = lrcoef_new_skewtab(outer, inner, part_length(content) as i32).into_boxed_slice();
	#[allow(non_snake_case)]
	let mut C = lrcoef_new_content(content).into_boxed_slice();

	let n = iv_sum(content);
	let mut pos = 0;
	let mut b = unsafe { &mut *T.as_mut_ptr() };
	let mut coef = 0i64;
	let mut above = T[b.north as usize].value;
	let mut x = 1;
	let mut se_supply = n - C[1].supply;

	loop {
		while x > above
			&& (C[x as usize].cont == C[x as usize].supply
				|| C[x as usize].cont == C[(x - 1) as usize].cont)
		{
			se_supply += C[x as usize].supply - C[x as usize].cont;
			x -= 1;
		}

		if x == above || n - pos - se_supply <= b.west_sz {
			pos -= 1;
			if pos < 0 {
				break;
			}
			b = unsafe { &mut *((b as *mut LRCoefBox).offset(-1)) };
			se_supply = b.se_supply;
			above = T[b.north as usize].value;
			x = b.value;
			C[x as usize].cont -= 1;
			se_supply += C[x as usize].supply - C[x as usize].cont;
			x -= 1;
		} else if pos + 1 < n {
			b.se_supply = se_supply;
			b.value = x;
			C[x as usize].cont += 1;
			pos += 1;
			b = unsafe { &mut *((b as *mut LRCoefBox).offset(1)) };
			se_supply = T[b.east as usize].se_supply;
			x = T[b.east as usize].value;
			above = T[b.north as usize].value;
			while x > b.max {
				se_supply += C[x as usize].supply - C[x as usize].cont;
				x -= 1;
			}
			while x > above && se_supply < b.se_sz {
				se_supply += C[x as usize].supply - C[x as usize].cont;
				x -= 1;
			}
		} else {
			coef += 1;
			pos -= 1;
			b = unsafe { &mut *((b as *mut LRCoefBox).offset(-1)) };
			se_supply = b.se_supply;
			above = T[b.north as usize].value;
			x = b.value;
			C[x as usize].cont -= 1;
			se_supply += C[x as usize].supply - C[x as usize].cont;
			x -= 1;
		}
	}
	coef
}
