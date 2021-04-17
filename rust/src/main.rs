use lrcalc::{coprod, lrcoef, mult, skew};

fn main() {
    let a = [91, 84, 70, 56, 42, 28, 21];
    let b = [49, 42, 35, 28, 21, 14, 7];
    let c = [49, 42, 35, 28, 21, 14, 7];
    println!(
        "lrcoef: outer = {:?}, inner_1 = {:?}, inner_2 = {:?}",
        a, b, c
    );
    let ans = lrcoef(&a, &b, &c);
    println!("{}", ans);

    let a = [3, 2, 1];
    let b = [3, 2, 1];
    println!("mult: sh1 = {:?}, sh2 = {:?}", a, b);
    let ans = mult(&a, &b, None, None);
    println!("{:?}", ans);

    let a = [3, 2, 1];
    let b = [2, 1];
    println!("skew: sh1 = {:?}, sh2 = {:?}", a, b);
    let ans = skew(&a, &b, Some(2));
    println!("{:?}", ans);

    let a = [3, 2, 1];
    println!("coprod: sh = {:?}", a);
    let ans = coprod(&a, None);
    println!("{:?}", ans);
}
