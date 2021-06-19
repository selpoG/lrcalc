# Python bindings for the Littlewood-Richardson Calculator

Then compile the bindings module:

```sh
cargo build
cp target/debug/liblrcalc.so lrcalc.so
```

Test that it works as follows:

```sh
python3 -c "import lrcalc; print(lrcalc.mult((2, 1), (2, 1)))"
```

## Test

```sh
python -m unittest
```
