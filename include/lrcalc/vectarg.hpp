#ifndef LRCALC_VECTARG_H
#define LRCALC_VECTARG_H

#include "lrcalc/ivector.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
	ivector* get_vect_arg(int ac, const char* const* av);
#ifdef __cplusplus
}
#endif

#endif
