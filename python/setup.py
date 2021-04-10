#!/usr/bin/python3

from setuptools import Extension, setup
from Cython.Build import cythonize

setup(name='lrcalc',
    version='2.0.0',
    description='Littlewood-Richardson Calculator',
    long_description='Littlewood-Richardson Calculator',
    url='https://math.rutgers.edu/~asbuch/lrcalc',
    author='Anders Skovsted Buch',
    license='GPL3',
    ext_modules = cythonize([
        Extension("lrcalc", ["lrcalc.pyx"],
                  libraries=["lrcalc"]),
    ], language_level="3"),
)
