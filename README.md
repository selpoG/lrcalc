# lrcalc

- [lrcalc](#lrcalc)
  - [Build](#build)
  - [Run](#run)
  - [Test](#test)

## Build

```sh
cargo build
```

## Run

```sh
$ cargo run --bin lrcalc lrcoef 91,84,70,56,42,28,21 49,42,35,28,21,14,7 49,42,35,28,21,14,7
2162400
```

## Test

```sh
cargo test
BIN_DIR=./target/debug ./testsuite
```
