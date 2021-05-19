/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/maple.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/part.hpp"

void maple_print_lincomb(const ivlincomb* ht, const char* letter, bool nz)
{
	putchar_r('0');
	for (const auto& kv : ivlc_iterator(ht))
	{
		if (kv.value == 0) continue;
		_maple_print_term(kv.value, kv.key, letter, nz);
	}
	putchar_r('\n');
}

void maple_qprint_lincomb(const ivlincomb* lc, int level, const char* letter)
{
	putchar_r('0');
	for (const auto& kv : ivlc_iterator(lc))
	{
		if (kv.value == 0) continue;
		_maple_qprint_term(kv.value, kv.key, level, letter);
	}
	putchar_r('\n');
}
