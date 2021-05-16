use std::fmt::{self, Display, Formatter};

use anyhow::{ensure, Context, Result};
use clap::AppSettings::{AllowLeadingHyphen, DeriveDisplayOrder};
use clap::{crate_version, Clap};

use lrcalc::{are_compatible_strs, is_permutation, schubmult, schubmult_str};

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
        if self.0.len() == 0 {
            return Ok(());
        }
        write!(f, "{}", self.0[0])?;
        for i in 1..self.0.len() {
            write!(f, ",{}", self.0[i])?
        }
        Ok(())
    }
}

struct LinearCombinationFormatter<'a> {
    data: &'a [(Vec<i32>, i32)],
    maple: bool,
}

impl<'a> Display for LinearCombinationFormatter<'a> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        if self.maple {
            write!(f, "0")?;
            for (sh, n) in self.data {
                if *n == 0 {
                    continue;
                }
                write!(f, "{:+}*X[{}]", n, VecFormatter(sh))?;
            }
            writeln!(f)
        } else {
            for (sh, n) in self.data {
                if *n == 0 {
                    continue;
                }
                writeln!(f, "{}  ({})", n, VecFormatter(sh))?;
            }
            Ok(())
        }
    }
}

#[derive(Clap)]
#[clap(name="schubmult", version = crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct Opts {
    /// Print in maple format
    #[clap(long = "--maple", short = 'm')]
    maple: bool,
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
            are_compatible_strs(&opts.perm1, &opts.perm2),
            "strs are not compatible"
        );
        schubmult_str(&opts.perm1, &opts.perm2)
    } else {
        assert!(is_permutation(&opts.perm1), "perm1 is not a permutation");
        assert!(is_permutation(&opts.perm2), "perm2 is not a permutation");
        schubmult(&opts.perm1, &opts.perm2, opts.rank)
    };
    print!(
        "{}",
        LinearCombinationFormatter {
            data: &lc,
            maple: opts.maple
        }
    )
}
