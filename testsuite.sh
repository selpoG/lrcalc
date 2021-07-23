#!/bin/sh
set -eu

failed=0
check () {
    command="$1"
    expected="$2"
    echo -n "testing $command ... "
    result=`$command`
    if [ "$result" != "$expected" ]; then
        echo "\033[31mfailed:\033[0m"
        echo "\033[31m  Expected: $expected\033[0m"
        echo "\033[31m  Got: $result\033[0m"
        failed=1
    else
        echo "\033[32mok\033[0m"
    fi
}

check "cargo run -q --bin lrcalc -- lrcoef 3,2,1 2,1 2,1" "2"

check "cargo run -q --bin lrcalc -- skew 3,2,1 2,1" "1  (3)
2  (2,1)
1  (1,1,1)"

check "cargo run -q --bin lrcalc -- skew -r 2 3,2,1 2,1" "2  (2,1)
1  (3)"

check "cargo run -q --bin lrcalc -- mult 2,1 2,1" "1  (4,2)
1  (3,3)
1  (4,1,1)
2  (3,2,1)
1  (3,1,1,1)
1  (2,2,1,1)
1  (2,2,2)"

check "cargo run -q --bin lrcalc -- mult -f -r 3 -c 2 3,2,1 3,2,1" "1  (5,4,3)
1  (4,4,4)"

check "cargo run -q --bin lrcalc -- mult -q -r 3 -c 2 3,2,1 3,2,1" "1  (1,1)
1  (2)"

check "cargo run -q --bin lrcalc -- coprod 3,2,1" "1  (3,2,1)  ()
1  (3,2)  (1)
1  (3,1,1)  (1)
1  (3,1)  (2)
1  (2,2,1)  (1)
1  (2,2)  (2)
1  (2,1,1)  (2)
1  (2,1)  (3)
1  (3,1)  (1,1)
1  (2,2)  (1,1)
2  (2,1)  (2,1)
1  (2,1,1)  (1,1)
1  (1,1,1)  (2,1)"

check "cargo run -q --bin schubmult -- 1,3,2 1,3,2" "1  (1,4,2,3)
1  (2,3,1)"

check "cargo run -q --bin schubmult -- -s 0,1,2,0,1,2 0,1,2,0,1,2" "1  (1,0,2,2,0,1)
1  (2,0,1,1,0,2)
1  (0,1,2,2,1,0)
1  (2,1,0,0,1,2)
1  (0,2,1,1,2,0)
1  (1,2,0,0,2,1)"

check "cargo run -q --bin allperms -- 0" "()"

check "cargo run -q --bin allperms -- 1" "(1)"

check "cargo run -q --bin allperms -- 3" "(1,2,3)
(1,3,2)
(2,1,3)
(2,3,1)
(3,1,2)
(3,2,1)"

exit $failed
