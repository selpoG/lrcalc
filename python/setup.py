#!/usr/bin/python3

from pathlib import Path

from Cython.Build import cythonize
from setuptools import Extension, setup

_here = Path(__file__).parent
setup(
    name="lrcalc",
    version="2.0.0",
    description="Littlewood-Richardson Calculator",
    long_description="Littlewood-Richardson Calculator",
    url="https://math.rutgers.edu/~asbuch/lrcalc",
    author="Anders Skovsted Buch",
    license="GPL3",
    ext_modules=cythonize(
        [
            Extension(
                "lrcalc",
                ["lrcalc.pyx"],
                include_dirs=[str(_here.parent.absolute() / "include")],
                library_dirs=[str(_here.absolute())],
                language="c++",
                libraries=["lrcalc"],
            ),
        ],
        language_level="3",
    ),
)
