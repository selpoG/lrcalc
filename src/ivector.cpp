/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <limits.h>

#include "lrcalc/alloc.hpp"

#define LRCALC_IVECTOR_C
#include "lrcalc/ivector.hpp"

#define MINVALUE INT_MIN
#define MAXVALUE INT_MAX
#define INTEGER_VALUE
#define VA_VALUE_T int
#define VALUE_FMT "%d"

#include "vector.tpl.cpp"
