use std::fmt::{self, Display, Formatter};

use anyhow::{ensure, Context, Result};
use clap::AppSettings::{AllowLeadingHyphen, DeriveDisplayOrder};
use clap::{crate_version, Clap};

use lrcalc::all_parts;

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

#[derive(Clap)]
#[clap(name="allparts", version = crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct Opts {
    #[clap(validator=check_non_negative, index=1, required=true)]
    rows: i32,
    #[clap(validator=check_non_negative, index=2, required=true)]
    cols: i32,
}

fn main() {
    let opts = Opts::parse();
    for perm in all_parts(opts.rows, opts.cols) {
        println!("({})", VecFormatter(&perm))
    }
}
