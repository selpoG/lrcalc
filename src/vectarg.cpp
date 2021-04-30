/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/vectarg.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
extern int optind;

#include <new>

#include "lrcalc/ivector.hpp"

ivector* get_vect_arg(int ac, const char* const* av)
{
	if (optind == ac) return nullptr;

	if (optind == 0)
		optind++;
	else
	{
		/* skip any "-" or "/" argument */
		char ch = *(av[optind]);
		if ((ch == '-' || ch == '/') && *(av[optind] + 1) == '\0') optind++;
	}

	auto tmp = new (std::nothrow) int[uint32_t(ac - optind)];
	if (tmp == nullptr) return nullptr;

	uint32_t n = 0;
	while (optind < ac)
	{
		char* endptr;
		auto x = int(strtol(av[optind], &endptr, 10));
		if (endptr == av[optind] || *endptr != '\0') break;

		tmp[n++] = x;
		optind++;
	}

	if (n == 0) return nullptr;

	ivector* res = iv_new(n);
	if (res == nullptr) return nullptr;
	for (uint32_t i = 0; i < n; i++) iv_elem(res, i) = tmp[i];
	delete[] tmp;

	return res;
}
