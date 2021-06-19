#![allow(clippy::not_unsafe_ptr_arg_deref)]
use super::ivector::{iv_free_ptr, IntVector};

pub struct IntVectorList {
    pub array: *mut *mut IntVector,
    pub allocated: u64,
    pub length: u64,
}

fn _ivl_require(lst: &mut IntVectorList, size: usize) {
    if lst.allocated as usize >= size {
        return;
    }
    let new_size = 2 * size;

    let mut vec = vec![std::ptr::null_mut(); new_size];
    let s = unsafe { std::slice::from_raw_parts_mut(lst.array, lst.allocated as usize) };
    let len = lst.length as usize;
    vec[..len].copy_from_slice(&s[..len]);
    let s = s.as_mut_ptr();
    unsafe { drop(Box::from_raw(s)) }

    lst.allocated = new_size as u64;
    let mut buf = vec.into_boxed_slice();
    lst.array = buf.as_mut_ptr();
    std::mem::forget(buf);
}

pub fn ivl_free_all(lst: *mut IntVectorList) {
    if lst.is_null() {
        return;
    }
    if unsafe { (*lst).allocated } != 0 {
        let lst = unsafe { &*lst };
        let s = unsafe { std::slice::from_raw_parts_mut(lst.array, lst.allocated as usize) };
        for v in s[..lst.length as usize].iter() {
            iv_free_ptr(*v as *mut IntVector)
        }
        let s = s.as_mut_ptr();
        unsafe { drop(Box::from_raw(s)) }
    }
    unsafe { drop(Box::from_raw(lst)) }
}
