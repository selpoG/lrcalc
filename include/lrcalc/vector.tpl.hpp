#include <stdlib.h>
#include <string.h>

#include "lrcalc/alloc.hpp"

typedef struct
{
	SIZE_T length;
	VALUE_T array[];
} VECTOR;

#ifdef DEBUG
VALUE_T* PREFIX(pelem)(VECTOR* v, SIZE_T i);
#endif

VECTOR* PREFIX(new)(SIZE_T length);

VECTOR* PREFIX(new_zero)(SIZE_T length);

VECTOR* PREFIX(new_init)(SIZE_T length, ...);

void PREFIX(free)(VECTOR* v);

VECTOR* PREFIX(new_copy)(const VECTOR* v);

void PREFIX(set_zero)(VECTOR* v);

void PREFIX(copy)(VECTOR* d, const VECTOR* s);

int PREFIX(cmp)(const VECTOR* v1, const VECTOR* v2);

#ifdef INTEGER_VALUE
uint32_t PREFIX(hash)(const VECTOR* v);
#endif

VALUE_T PREFIX(sum)(const VECTOR* v);
int PREFIX(lesseq)(const VECTOR* v1, const VECTOR* v2);
void PREFIX(mult)(VECTOR* dst, VALUE_T c, const VECTOR* src);
void PREFIX(div)(VECTOR* dst, const VECTOR* src, VALUE_T c);
VALUE_T PREFIX(max)(const VECTOR* v);
VALUE_T PREFIX(min)(const VECTOR* v);
void PREFIX(reverse)(VECTOR* dst, const VECTOR* src);

#ifdef INTEGER_VALUE
VALUE_T PREFIX(gcd)(const VECTOR* v);
#endif

void PREFIX(print)(const VECTOR* v);
void PREFIX(printnl)(const VECTOR* v);
