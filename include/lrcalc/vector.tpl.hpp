#include <stdlib.h>
#include <string.h>

#include "lrcalc/alloc.hpp"

typedef struct
{
	SIZE_T length;
	VALUE_T array[];
} VECTOR;

#ifdef DEBUG
INLINE VALUE_T* PREFIX(pelem)(VECTOR* v, SIZE_T i)
{
	claim(i >= 0);
	claim(i < v->length);
	return v->array + i;
}
#endif

INLINE VECTOR* PREFIX(new)(SIZE_T length)
{
	auto v = static_cast<VECTOR*>(ml_malloc(sizeof(VECTOR) + length * sizeof(VALUE_T)));
	if (v == nullptr) return nullptr;
	v->length = length;
	return v;
}

INLINE VECTOR* PREFIX(new_zero)(SIZE_T length)
{
	auto v = static_cast<VECTOR*>(ml_calloc(1, sizeof(VECTOR) + length * sizeof(VALUE_T)));
	if (v == nullptr) return nullptr;
	v->length = length;
	return v;
}

VECTOR* PREFIX(new_init)(SIZE_T length, ...);

INLINE void PREFIX(free)(VECTOR* v) { ml_free(v); }

INLINE VECTOR* PREFIX(new_copy)(VECTOR* v)
{
	VECTOR* vc = PREFIX(new)(v->length);
	if (vc == nullptr) return nullptr;
	memcpy(vc->array, v->array, v->length * sizeof(VALUE_T));
	return vc;
}

INLINE void PREFIX(set_zero)(VECTOR* v) { memset(v->array, 0, v->length * sizeof(VALUE_T)); }

INLINE void PREFIX(copy)(VECTOR* d, VECTOR* s)
{
	claim(d->length == s->length);
	memcpy(d->array, s->array, d->length * sizeof(VALUE_T));
}

INLINE int PREFIX(cmp)(VECTOR* v1, VECTOR* v2)
{
	if (v1->length != v2->length) return int(v1->length) - int(v2->length);
	for (SIZE_T i = 0; i < v1->length; i++)
		if (v1->array[i] != v2->array[i]) return v1->array[i] - v2->array[i];
	return 0;
}

#ifdef INTEGER_VALUE
INLINE uint32_t PREFIX(hash)(VECTOR* v)
{
	uint32_t h = v->length;
	for (SIZE_T i = 0; i < v->length; i++) h = ((h << 5) ^ (h >> 27)) + uint32_t(v->array[i]);
	return h;
}
#endif

VALUE_T PREFIX(sum)(VECTOR* v);
int PREFIX(lesseq)(VECTOR* v1, VECTOR* v2);
void PREFIX(mult)(VECTOR* dst, VALUE_T c, VECTOR* src);
void PREFIX(div)(VECTOR* dst, VECTOR* src, VALUE_T c);
VALUE_T PREFIX(max)(VECTOR* v);
VALUE_T PREFIX(min)(VECTOR* v);
void PREFIX(reverse)(VECTOR* dst, VECTOR* src);

#ifdef INTEGER_VALUE
VALUE_T PREFIX(gcd)(VECTOR* v);
#endif

void PREFIX(print)(VECTOR* v);
void PREFIX(printnl)(VECTOR* v);
