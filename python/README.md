# Python bindings for the Littlewood-Richardson Calculator

## Install by wheel (Python >= 3.6)

```sh
python -m pip install maturin
maturin build -i python
pip install target/wheels/<built .whl>
```

## Install by pip (Python >= 3.6, pip >= 21.1)

```sh
pip install . --use-feature=in-tree-build
```

## Build (For developers)

First, create a virtualenv using `poetry`:

```sh
python -m pip install poetry
potery install
poetry shell
```

Then compile the bindings module:

```sh
maturin develop
```

Test that it works as follows:

```sh
python -c "import lrcalc; print(lrcalc.mult((2, 1), (2, 1)))"
```

## Test

```sh
python -m unittest
```
