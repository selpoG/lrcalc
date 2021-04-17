# lrcalc-rs

## Build

You must install `lrcalc` first.

```sh
bindgen src/wrapper.h > src/lrcalc/bindings.rs
RUSTFLAGS="-llrcalc" cargo build
```
