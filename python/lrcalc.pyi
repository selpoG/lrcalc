"""Python bindings for the Littlewood-Richardson Calculator."""

from typing import Dict, Iterator, Literal, Sequence, Tuple, overload

def lrcoef(out: Sequence[int], inn1: Sequence[int], inn2: Sequence[int]) -> int:
    """Compute a single Littlewood-Richardson coefficient."""
    ...

def mult(
    sh1: Sequence[int], sh2: Sequence[int], rows: int = -1, cols: int = -1
) -> Dict[Tuple[int, ...], int]:
    """Compute the product of two Schur functions."""
    ...

def mult_fusion(
    sh1: Sequence[int], sh2: Sequence[int], rows: int, level: int
) -> Dict[Tuple[int, ...], int]:
    """Compute a product in the fusion ring of type A."""
    ...

@overload
def mult_quantum(
    sh1: Sequence[int], sh2: Sequence[int], rows: int, cols: int
) -> Dict[Tuple[int, ...], int]:
    """Compute quantum product of Schubert classes on a Grassmannian."""
    ...

@overload
def mult_quantum(
    sh1: Sequence[int],
    sh2: Sequence[int],
    rows: int,
    cols: int,
    degrees: Literal[False],
) -> Dict[Tuple[int, ...], int]:
    """Compute quantum product of Schubert classes on a Grassmannian."""
    ...

@overload
def mult_quantum(
    sh1: Sequence[int], sh2: Sequence[int], rows: int, cols: int, degrees: Literal[True]
) -> Dict[Tuple[Tuple[int, ...], int], int]:
    """Compute quantum product of Schubert classes on a Grassmannian."""
    ...

def skew(
    outer: Sequence[int], inner: Sequence[int], rows: int = -1
) -> Dict[Tuple[int, ...], int]:
    """Compute the Schur expansion of a skew Schur function."""
    ...

def coprod(
    sh: Sequence[int], all: bool = False
) -> Dict[Tuple[Tuple[int, ...], Tuple[int, ...]], int]:
    """Compute the coproduct of a Schur function."""
    ...

def schubert_poly(w: Sequence[int]) -> Dict[Tuple[int, ...], int]:
    """Compute the Schubert polynomial of a permutation."""
    ...

def schubmult(
    w1: Sequence[int], w2: Sequence[int], rank: int = 0
) -> Dict[Tuple[int, ...], int]:
    """Compute the product of two Schubert polynomials."""
    ...

def schubmult_str(
    str1: Sequence[int], str2: Sequence[int]
) -> Dict[Tuple[int, ...], int]:
    """Compute product of Schubert polynomials using string notation."""
    ...

def lr_iterator(
    self, outer: Sequence[int], inner: Sequence[int], rows: int = -1
) -> Iterator[Tuple[int, ...]]: ...
