# lrcalc-rs

## Build

You must build `lrcalc` first.

```sh
bindgen src/wrapper.hpp > src/lrcalc/bindings.rs -- -I../include
cp /path/to/liblrcalc.a .
RUSTFLAGS="-L$(pwd) -llrcalc -lstdc++" cargo build
```

## Run

```sh
$ ./target/release/lrcalc lrcoef 91,84,70,56,42,28,21 49,42,35,28,21,14,7 49,42,35,28,21,14,7
2162400
```

## Test

```sh
RUSTFLAGS="-L$(pwd) -llrcalc -lstdc++" cargo test
BIN_DIR=./target/debug ./testsuite
```
