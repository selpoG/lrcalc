#ifndef LRCALC_MAPLE_H
#define LRCALC_MAPLE_H

void maple_print_lincomb(const ivlincomb* ht, const char* letter, int nz);

void maple_qprint_lincomb(const ivlincomb* lc, int level, const char* letter);

#endif
