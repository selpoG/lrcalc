#!/bin/bash

# Instructions:
#
# ./configure CFLAGS="-DDEBUG"
# make clean; make check
# cd tests
# ./oomsuite.sh
#
# See also oomtest.sh.


oomtest()
{
    echo "./oomtest.sh $@"
    ./oomtest.sh "$@"
}

oomtest ../src/lrcalc mult 0 - 0
oomtest ../src/lrcalc mult -m 0 - 0
oomtest ../src/lrcalc mult -q1,1 0 - 0
oomtest ../src/lrcalc mult -f1,1 0 - 0
oomtest ../src/lrcalc mult 1 - 0
oomtest ../src/lrcalc mult 0 - 1
oomtest ../src/lrcalc mult 2 1 - 2 1
oomtest ../src/lrcalc mult -m 2 1 - 2 1
oomtest ../src/lrcalc mult -r2 2 1 - 2 1
oomtest ../src/lrcalc mult -c2 2 1 - 2 1
oomtest ../src/lrcalc mult -r3 -c2 2 1 - 2 1
oomtest ../src/lrcalc mult -f2,2 2 1 - 2 1
oomtest ../src/lrcalc mult -m -f2,2 2 1 - 2 1
oomtest ../src/lrcalc mult -q2,2 2 1 - 2 1
oomtest ../src/lrcalc mult -m -q2,2 2 1 - 2 1
oomtest ../src/lrcalc mult 4 2 1 - 5 3 1
oomtest ../src/lrcalc mult -q4,4 4 3 1 - 5 5 1 1
oomtest ../src/lrcalc mult -f4,5 5 3 1 - 4 2 1

oomtest ../src/lrcalc skew 0 / 0
oomtest ../src/lrcalc skew -m 0 / 0
oomtest ../src/lrcalc skew 3 1 / 2 2
oomtest ../src/lrcalc skew -m 3 1 / 2 2
oomtest ../src/lrcalc skew -r0 3 2 1 / 1
oomtest ../src/lrcalc skew -m -r0 3 2 1 / 1
oomtest ../src/lrcalc skew -r1 3 2 1 / 1
oomtest ../src/lrcalc skew -r2 3 2 1 / 1
oomtest ../src/lrcalc skew -r3 3 2 1 / 1
oomtest ../src/lrcalc skew 6 4 3 3 1 / 3 2 1
oomtest ../src/lrcalc skew -m 6 4 3 3 1 / 3 2 1
oomtest ../src/lrcalc skew -r4 6 4 3 3 1 / 3 2 1

oomtest ../src/lrcalc coprod 0
oomtest ../src/lrcalc coprod -a 0
oomtest ../src/lrcalc coprod 1
oomtest ../src/lrcalc coprod -a 1
oomtest ../src/lrcalc coprod 2 1
oomtest ../src/lrcalc coprod -a 2 1
oomtest ../src/lrcalc coprod 4 3 2 1
oomtest ../src/lrcalc coprod -a 4 3 2 1

oomtest ../src/lrcalc coef 0 - 0 - 0
oomtest ../src/lrcalc coef 1 - 0 - 0
oomtest ../src/lrcalc coef 0 - 0 - 1
oomtest ../src/lrcalc coef 2 2 1 - 3 - 2
oomtest ../src/lrcalc coef 2 1 - 2 - 1
oomtest ../src/lrcalc coef 5 4 3 2 1 - 3 2 1 - 4 3 1 1
oomtest ../src/lrcalc coef 7 6 5 4 3 2 1 - 4 4 3 2 1 - 5 4 3 2

oomtest ../src/lrcalc tab 0 / 0
oomtest ../src/lrcalc tab 0 / 1
oomtest ../src/lrcalc tab 1 / 0
oomtest ../src/lrcalc tab 3 1 / 2 2
oomtest ../src/lrcalc tab 2 1 / 1
oomtest ../src/lrcalc tab -r0 2 1 / 1
oomtest ../src/lrcalc tab -r0 2 1 / 1
oomtest ../src/lrcalc tab -r0 2 1 / 1
oomtest ../src/lrcalc tab 4 3 2 1 / 2 1
oomtest ../src/lrcalc tab -r0 4 3 2 1 / 2 1
oomtest ../src/lrcalc tab -r1 4 3 2 1 / 2 1
oomtest ../src/lrcalc tab -r2 4 3 2 1 / 2 1
oomtest ../src/lrcalc tab -r3 4 3 2 1 / 2 1
oomtest ../src/lrcalc tab -r4 4 3 2 1 / 2 1

oomtest ../src/schubmult 1 - 1
oomtest ../src/schubmult 1 - 1 2
oomtest ../src/schubmult 2 1 - 2 1
oomtest ../src/schubmult 1 3 2 - 1 3 2
oomtest ../src/schubmult 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -r1 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -m -r1 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -r6 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -r7 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -m -r7 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult -r8 2 4 6 1 3 5 - 2 4 6 1 3 5
oomtest ../src/schubmult 3 1 5 2 4 - 1 4 2 6 3 5

oomtest ../src/schubmult -s 0 - 0
oomtest ../src/schubmult -s -m 0 - 0
oomtest ../src/schubmult -s 3 - 3
oomtest ../src/schubmult -s -m 3 - 3
oomtest ../src/schubmult -s 1 2 - 2 1
oomtest ../src/schubmult -s -m 1 2 - 2 1
oomtest ../src/schubmult -s 1 0 - 1 0
oomtest ../src/schubmult -s -m 1 0 - 1 0
oomtest ../src/schubmult -s 0 1 2 0 1 2 - 0 1 2 0 1 2
oomtest ../src/schubmult -s -m 0 1 2 0 1 2 - 0 1 2 0 1 2
oomtest ../src/schubmult -s -m 0 2 1 3 1 0 3 2 - 1 2 0 2 1 3 0 3
oomtest ../src/schubmult -s 0 2 1 3 1 0 3 2 3 - 1 2 0 2 3 3 1 0 3

oomtest ../src/allperms 0
oomtest ../src/allperms 1
oomtest ../src/allperms 2
oomtest ../src/allperms 3
oomtest ../src/allperms 4

oomtest ../src/allstrings 0
oomtest ../src/allstrings 0 0 0 0
oomtest ../src/allstrings 1
oomtest ../src/allstrings 0 0 0 1 1 1 1 2 2 2
oomtest ../src/allstrings 0 0 3 3 3 3 4 4 6 6

oomtest ../src/test_partiter 1 1
oomtest ../src/test_partiter 2 1
oomtest ../src/test_partiter 1 2
oomtest ../src/test_partiter 3 2
oomtest ../src/test_partiter 2 3
