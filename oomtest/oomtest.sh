#!/bin/bash

# Out-Of-Memory Test
#
# This script is used to test that the lrcalc library does not leak
# memory, even when an out-of-memory event happens in the middle of
# a calculation.
#
# Instructions:
#
# ./configure CFLAGS="-DDEBUG"
# make clean; make
# cd tests
# ./oomtest.sh ../src/lrcalc mult 1 - 1
# ./oomtest.sh ../src/schubmult 2 1 - 2 1
#
# Any output from oomtest.sh will indicate a problem.

verbose=0
if [[ $1 = "-v" ]]; then
    verbose=1
    shift
fi

if [[ $# = 0 ]]; then
    echo "usage: oomtest.sh [-v] command [options ...]" 1>&2
    exit 1
fi

failnum=1
while true; do
    out=$(ML_FAIL_NUMBER=$failnum "$@" 2>&1)
    if [[ $verbose != 0 ]]; then
        echo "----"
        echo "ML_FAIL_NUMBER=$failnum $@"
        echo "$out"
    fi
    err=""
    if [[ -z $out ]]; then
        err="no output"
    fi;
    if [[ -z $err ]]; then
        err=$(echo "$out" | grep "malloc called after out-of-memory")
    fi
    if [[ -z $err ]]; then
        err=$(echo "$out" | egrep "Memory balance: [^0]")
    fi
    if [[ -n $err ]]; then
        echo "$failnum: $err"
    fi
    if [[ -z $err ]]; then
        err=$(echo "$out" | grep "out of memory")
    fi;
    bal=$(echo "$out" | grep "Memory balance: 0")
    if [[ -z "$err" && -n $bal ]]; then
        break
    fi
    failnum=$((failnum + 1))
done
