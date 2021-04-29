/*
 * List type for elements of user defined types.
 *
 * Macros used:
 *
 * LIST           :  Name of list type
 * PREFIX(name)   :  Returns e.g. l_name
 * VALUE_T        :  Type of list elements, e.g. void *
 * SIZE_T         :  Type for lengths, e.g. size_t
 */

#include <stdlib.h>
#include <string.h>

#include "lrcalc/alloc.hpp"

typedef struct
{
	VALUE_T* array;
	SIZE_T allocated;
	SIZE_T length;
} LIST;

#ifdef DEBUG
VALUE_T* PREFIX(pelem)(LIST* lst, SIZE_T i);
#endif

/* Initialize list structure. */
int PREFIX(init)(LIST* lst, SIZE_T sz);

LIST* PREFIX(new)(SIZE_T sz);

LIST* PREFIX(new_init)(SIZE_T sz, SIZE_T count, ...);

void PREFIX(dealloc)(LIST* v);

void PREFIX(free)(LIST* v);

void PREFIX(reset)(LIST* lst);

int PREFIX(_realloc_array)(LIST* lst, SIZE_T sz);

int PREFIX(makeroom)(LIST* lst, SIZE_T sz);

int PREFIX(append)(LIST* lst, VALUE_T x);

VALUE_T PREFIX(poplast)(LIST* lst);

int PREFIX(insert)(LIST* lst, SIZE_T i, VALUE_T x);

VALUE_T PREFIX(delete)(LIST* lst, SIZE_T i);

VALUE_T PREFIX(fastdelete)(LIST* lst, SIZE_T i);

int PREFIX(extend)(LIST* dst, const LIST* src);

int PREFIX(copy)(LIST* dst, const LIST* src);

LIST* PREFIX(new_copy)(const LIST* lst);

int PREFIX(reverse)(LIST* dst, const LIST* src);

/* eof */
