#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"

$failed = 0
function check {
	param ([string]$command, [string]$expected)

	Write-Host -NoNewLine "testing $command ... "
	$result = Invoke-Expression $command
	$result = $result -join "`n"
	if ($result -ne $expected) {
		Write-Host -ForegroundColor Red "failed:"
		Write-Host -ForegroundColor Red "  Expected: $expected"
		Write-Host -ForegroundColor Red "  Got: $result"
		$script:failed = 1
	}
	else {
		Write-Host -ForegroundColor Green "ok"
	}
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
