# Python bindings for the Littlewood-Richardson Calculator

First install the LR Calculator (or copy `liblrcalc.a` here).
Make sure that Python3 anc Cython3 are also installed.
Then compile the bindings module:

```sh
python3 setup.py build_ext --inplace
```

Test that it works as follows:

```sh
python3 -c "import lrcalc; print(lrcalc.mult((2, 1), (2, 1)))"
```
