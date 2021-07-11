use std::fmt::{self, Display, Formatter};

use anyhow::{ensure, Context, Result};
use clap::AppSettings::{AllowLeadingHyphen, DeriveDisplayOrder};
use clap::{crate_version, Clap};

use lrcalc::{is_permutation, perm::str_iscompat, schubmult, schubmult_str};

fn check_non_negative(num_str: &str) -> Result<()> {
    let num = num_str
        .parse::<i32>()
        .context("could not parse as an integer")?;
    ensure!(num >= 0, "must be non-negative");
    Ok(())
}

struct VecFormatter<'a>(&'a Vec<i32>);

impl<'a> Display for VecFormatter<'a> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        if self.0.is_empty() {
            return Ok(());
        }
        write!(f, "{}", self.0[0])?;
        for i in 1..self.0.len() {
            write!(f, ",{}", self.0[i])?
        }
        Ok(())
    }
}

struct LinearCombinationFormatter<'a>(&'a [(Vec<i32>, i32)]);

impl<'a> Display for LinearCombinationFormatter<'a> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        for (sh, n) in self.0 {
            if *n == 0 {
                continue;
            }
            writeln!(f, "{}  ({})", n, VecFormatter(sh))?;
        }
        Ok(())
    }
}

#[derive(Clap)]
#[clap(name="schubmult", version = crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct Opts {
    /// perm1, perm2 in string format
    #[clap(long = "--string", short = 's')]
    string: bool,
    #[clap(validator=check_non_negative, long = "--rank", short = 'r')]
    rank: Option<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    perm1: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=2, multiple=false)]
    perm2: Vec<i32>,
}

fn main() {
    let opts = Opts::parse();
    let lc = if opts.string {
        assert!(
            opts.rank.is_none(),
            "rank option cannot be used with string option"
        );
        assert!(
            str_iscompat(&opts.perm1, &opts.perm2),
            "strs are not compatible"
        );
        schubmult_str(&opts.perm1, &opts.perm2)
    } else {
        assert!(is_permutation(&opts.perm1), "perm1 is not a permutation");
        assert!(is_permutation(&opts.perm2), "perm2 is not a permutation");
        schubmult(&opts.perm1, &opts.perm2, opts.rank)
    };
    print!("{}", LinearCombinationFormatter(&lc))
}
