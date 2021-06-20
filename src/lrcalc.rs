use std::fmt::{self, Display, Formatter};

use anyhow::{ensure, Context, Result};
use clap::AppSettings::{AllowLeadingHyphen, DeriveDisplayOrder};
use clap::{crate_version, Clap};

use lrcalc::{coprod, is_partition, lrcoef, mult, mult_fusion, mult_quantum, skew, skew_tab};

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
#[clap(name="lrcalc", version = crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct Opts {
    #[clap(subcommand)]
    subcmd: SubCommand,
}

#[derive(Clap)]
enum SubCommand {
    Mult(MultOpts),
    Skew(SkewOpts),
    Coprod(CoprodOpts),
    #[clap(name = "lrcoef")]
    LRCoef(LRCoefOpts),
    Tab(TabOpts),
}

#[derive(Clap, Debug)]
#[clap(version=crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct MultOpts {
    /// Limit maximum rows allowed
    #[clap(long = "--rows", short = 'r')]
    rows: Option<i32>,
    /// Limit maximum cols allowed
    #[clap(long = "--cols", short = 'c')]
    cols: Option<i32>,
    /// If fusion is specified, cols is used as a level
    #[clap(long = "--fusion", short = 'f')]
    fusion: bool,
    #[clap(long = "--quantum", short = 'q')]
    quantum: bool,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    part1: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=2, multiple=false)]
    part2: Vec<i32>,
}

#[derive(Clap, Debug)]
#[clap(version=crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct SkewOpts {
    /// Limit maximum rows allowed
    #[clap(long = "--rows", short = 'r')]
    rows: Option<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    outer: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=2, multiple=false)]
    inner: Vec<i32>,
}

#[derive(Clap, Debug)]
#[clap(version=crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct CoprodOpts {
    /// Print all coprods including flipped pairs
    #[clap(long = "--all", short = 'a')]
    all: bool,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    part: Vec<i32>,
}

#[derive(Clap, Debug)]
#[clap(version=crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct LRCoefOpts {
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    outer: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=2, multiple=false)]
    inner1: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=3, multiple=false)]
    inner2: Vec<i32>,
}

#[derive(Clap, Debug)]
#[clap(version=crate_version!(), setting=AllowLeadingHyphen, setting=DeriveDisplayOrder)]
struct TabOpts {
    /// Limit maximum rows allowed
    #[clap(long = "--rows", short = 'r')]
    rows: Option<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=1, multiple=false)]
    outer: Vec<i32>,
    #[clap(validator=check_non_negative, value_delimiter=",", required=true, index=2, multiple=false)]
    inner: Vec<i32>,
}

fn main() {
    let opts = Opts::parse();
    match opts.subcmd {
        SubCommand::Mult(opts) => {
            assert!(is_partition(&opts.part1), "part1 is not a partition");
            assert!(is_partition(&opts.part2), "part2 is not a partition");
            if opts.quantum || opts.fusion {
                let rows = opts
                    .rows
                    .expect("rows is required in quantum or fusion mode");
                let cols = opts
                    .cols
                    .expect("cols is required in quantum or fusion mode");
                if opts.quantum {
                    print!(
                        "{}",
                        LinearCombinationFormatter(
                            &mult_quantum(&opts.part1, &opts.part2, rows, cols)
                                .into_iter()
                                .map(|((_, sh), n)| (sh, n))
                                .collect::<Vec<_>>()
                        )
                    )
                } else {
                    print!(
                        "{}",
                        LinearCombinationFormatter(&mult_fusion(
                            &opts.part1,
                            &opts.part2,
                            rows,
                            cols
                        ))
                    )
                }
            } else {
                print!(
                    "{}",
                    LinearCombinationFormatter(&mult(
                        &opts.part1,
                        &opts.part2,
                        opts.rows,
                        opts.cols
                    ))
                )
            }
        }
        SubCommand::Skew(opts) => {
            assert!(is_partition(&opts.outer), "outer is not a partition");
            assert!(is_partition(&opts.inner), "inner is not a partition");
            print!(
                "{}",
                LinearCombinationFormatter(&skew(&opts.outer, &opts.inner, opts.rows))
            )
        }
        SubCommand::Coprod(opts) => {
            assert!(is_partition(&opts.part), "part is not a partition");
            for (sh1, sh2, n) in coprod(&opts.part, Some(opts.all)) {
                println!("{}  ({})  ({})", n, VecFormatter(&sh1), VecFormatter(&sh2))
            }
        }
        SubCommand::LRCoef(opts) => {
            assert!(is_partition(&opts.outer), "outer is not a partition");
            assert!(is_partition(&opts.inner1), "inner1 is not a partition");
            assert!(is_partition(&opts.inner2), "inner2 is not a partition");
            println!("{}", lrcoef(&opts.outer, &opts.inner1, &opts.inner2));
        }
        SubCommand::Tab(opts) => {
            let outer = opts.outer;
            let inner = opts.inner;
            assert!(is_partition(&outer), "outer is not a partition");
            assert!(is_partition(&inner), "inner is not a partition");
            let ilen = inner.len();
            let mut len = outer.len();
            while len > 0 && outer[len - 1] == 0 {
                len -= 1
            }
            if len <= ilen {
                while len > 0 && inner[len - 1] == outer[len - 1] {
                    len -= 1
                }
            }
            let len = len;
            let col_first = if ilen < len { 0 } else { inner[len - 1] };
            let mut r_start = 0;
            while r_start < ilen && inner[r_start] == outer[r_start] {
                r_start += 1
            }
            for values in skew_tab(&outer, &inner, opts.rows) {
                let mut size = values.len() as i32;
                let width = std::cmp::max(
                    2,
                    values
                        .iter()
                        .max()
                        .map(|&m| m.to_string().len())
                        .unwrap_or(0),
                );
                for r in r_start..len {
                    let inn_r = if r >= ilen { 0 } else { inner[r] };
                    let out_r = outer[r];
                    let row_sz = out_r - inn_r;
                    size -= row_sz;
                    print!("{}", " ".repeat(width * (inn_r - col_first) as usize));
                    for c in 0..row_sz {
                        print!("{:width$}", values[(size + c) as usize], width = width)
                    }
                    println!()
                }
                println!()
            }
        }
    }
}
