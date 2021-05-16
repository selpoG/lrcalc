import unittest

from lrcalc import (
    coprod,
    lrcoef,
    mult,
    mult_fusion,
    mult_quantum,
    schubmult,
    schubmult_str,
    skew,
)


class LRCalcTestCase(unittest.TestCase):
    def test_lrcoef(self):
        self.assertEqual(lrcoef([3, 2, 1], [2, 1], [2, 1]), 2)

    def test_skew(self):
        self.assertDictEqual(
            skew([3, 2, 1], [2, 1]), {(2, 1): 2, (1, 1, 1): 1, (3,): 1}
        )
        self.assertDictEqual(skew([3, 2, 1], [2, 1], 2), {(2, 1): 2, (3,): 1})

    def test_mult(self):
        self.assertDictEqual(
            mult([2, 1], [2, 1]),
            {
                (3, 2, 1): 2,
                (4, 2): 1,
                (3, 1, 1, 1): 1,
                (3, 3): 1,
                (2, 2, 1, 1): 1,
                (2, 2, 2): 1,
                (4, 1, 1): 1,
            },
        )
        self.assertDictEqual(
            mult_fusion([3, 2, 1], [3, 2, 1], 3, 2), {(4, 4, 4): 1, (5, 4, 3): 1}
        )
        self.assertDictEqual(
            mult_quantum([3, 2, 1], [3, 2, 1], 3, 2), {(2,): 1, (1, 1): 1}
        )

    def test_coprod(self):
        self.assertDictEqual(
            coprod([3, 2, 1]),
            {
                ((2, 1, 1), (1, 1)): 1,
                ((3, 2, 1), ()): 1,
                ((3, 1, 1), (1,)): 1,
                ((2, 2), (1, 1)): 1,
                ((2, 1), (2, 1)): 2,
                ((3, 2), (1,)): 1,
                ((3, 1), (2,)): 1,
                ((1, 1, 1), (2, 1)): 1,
                ((2, 2, 1), (1,)): 1,
                ((2, 1, 1), (2,)): 1,
                ((2, 2), (2,)): 1,
                ((2, 1), (3,)): 1,
                ((3, 1), (1, 1)): 1,
            },
        )

    def test_schubmult(self):
        self.assertDictEqual(
            schubmult([1, 3, 2], [1, 3, 2]), {(2, 3, 1): 1, (1, 4, 2, 3): 1}
        )
        self.assertDictEqual(
            schubmult([1, 3, 4, 2], [2, 1, 4, 5, 3]),
            {
                (2, 3, 5, 4, 1): 1,
                (4, 1, 5, 2, 3): 1,
                (2, 4, 5, 1, 3): 1,
                (3, 2, 4, 5, 1): 1,
                (3, 1, 5, 4, 2): 1,
            },
        )
        self.assertDictEqual(
            schubmult_str([0, 1, 2, 0, 1, 2], [0, 1, 2, 0, 1, 2]),
            {
                (2, 1, 0, 0, 1, 2): 1,
                (1, 0, 2, 2, 0, 1): 1,
                (0, 2, 1, 1, 2, 0): 1,
                (2, 0, 1, 1, 0, 2): 1,
                (1, 2, 0, 0, 2, 1): 1,
                (0, 1, 2, 2, 1, 0): 1,
            },
        )
        self.assertDictEqual(
            schubmult_str([0, 2, 0, 2, 1, 2], [0, 1, 2, 0, 2, 2]),
            {
                (0, 2, 1, 2, 2, 0): 1,
                (2, 0, 1, 2, 0, 2): 1,
                (2, 0, 2, 0, 1, 2): 1,
                (0, 2, 2, 1, 0, 2): 1,
            },
        )


if __name__ == "__main__":
    unittest.main()
