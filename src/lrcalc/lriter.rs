use super::{
    ivector::{iv_hash, IntVector},
    ivlincomb::{ivlc_add_element, ivlc_new_default, LinearCombination, LC_COPY_KEY},
    part::{part_decr, part_length, part_leq, part_valid},
};

#[derive(Clone)]
struct LRIteratorBox {
    pub value: i32,
    max: i32,
    above: i32,
    right: i32,
}

pub struct LRTableauIterator {
    pub cont: IntVector,
    pub size: i32,
    array: Vec<LRIteratorBox>,
}

impl LRTableauIterator {
    pub fn new(
        outer: &IntVector,
        inner: Option<&IntVector>,
        content: Option<&IntVector>,
        mut maxrows: i32,
        maxcols: i32,
        mut partsz: i32,
    ) -> LRTableauIterator {
        debug_assert!(part_valid(&outer[..]));
        debug_assert!(inner.is_none() || part_valid(&inner.unwrap()[..]));
        debug_assert!(content.is_none() || part_decr(&content.unwrap()[..]));

        /* Empty result if inner not contained in outer. */
        if inner.is_some() && !part_leq(inner.unwrap(), outer) {
            return LRTableauIterator {
                cont: vec![0].into(),
                size: -1,
                array: Vec::new(),
            };
        }

        let len = part_length(&outer[..]) as u32;
        let outer = &outer[..];
        let inner = inner.map(|v| &v[..]);
        let mut ilen = inner.map(|v| v.len() as u32).unwrap_or(0);
        if ilen > len {
            ilen = len;
        }
        let clen = content.map(|v| part_length(&v[..]) as u32).unwrap_or(0);
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
        let mut arr = vec![
            LRIteratorBox {
                value: 0,
                max: 0,
                above: 0,
                right: 0
            };
            array_len as usize
        ];

        /* Allocate and copy content. */
        if partsz < maxrows {
            partsz = maxrows;
        }
        let partsz_u = partsz as u32;
        let mut cont = vec![0; partsz_u as usize];

        fn ret(size: i32, arr: Vec<LRIteratorBox>, cont: Vec<i32>) -> LRTableauIterator {
            LRTableauIterator {
                cont: cont.into(),
                size,
                array: arr,
            }
        }

        if maxrows < clen as i32 {
            return ret(-1, arr, cont);
        } /* empty result. */
        for r in 0..clen {
            cont[r as usize] = content.unwrap()[r as usize];
        }

        /* Check for empty result. */
        if maxcols >= 0 && clen > 0 && cont[0] > maxcols {
            return ret(-1, arr, cont);
        } /* empty result. */
        if maxcols >= 0 && out0 > maxcols {
            return ret(-1, arr, cont);
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
                return ret(-1, arr, cont);
            } /* empty result. */
            b.value = x;
            cont[x as usize] += 1;
        }

        ret(size, arr, cont)
    }
    pub fn is_good(&self) -> bool {
        self.size >= 0
    }
    pub fn get_array(&self) -> Vec<i32> {
        let array = &self.array[0..self.size as usize];
        array.iter().map(|b| b.value).collect()
    }
    pub fn next(&mut self) {
        let cont = &mut (self.cont)[..];
        let array = &mut self.array[..];
        let size = self.size;
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
        self.size = -1;
    }
    pub(crate) fn expand(
        outer: &IntVector,
        inner: Option<&IntVector>,
        content: Option<&IntVector>,
        maxrows: i32,
        maxcols: i32,
        partsz: i32,
    ) -> LinearCombination {
        let mut lrit_raw = LRTableauIterator::new(outer, inner, content, maxrows, maxcols, partsz);
        let mut lc = ivlc_new_default();
        while lrit_raw.is_good() {
            ivlc_add_element(
                &mut lc,
                1,
                lrit_raw.cont.clone(),
                iv_hash(&lrit_raw.cont[..]),
                LC_COPY_KEY,
            );
            lrit_raw.next();
        }
        lc
    }
}

impl Iterator for LRTableauIterator {
    type Item = Vec<i32>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.is_good() {
            let len = self.size;
            let arr = &self.array[..len as usize];
            let val = arr.iter().map(|x| x.value).collect();
            self.next();
            Some(val)
        } else {
            None
        }
    }
}
