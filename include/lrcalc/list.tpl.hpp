/*
 * List type for elements of user defined types.
 *
 * Macros used:
 *
 * LIST           :  Name of list type
 * PREFIX(name)   :  Returns e.g. l_name
 * VALUE_T        :  Type of list elements, e.g. void *
 */

#include <stdlib.h>
#include <string.h>

typedef struct
{
	VALUE_T* array;
	size_t allocated;
	size_t length;
} LIST;

/* Initialize list structure. */
int PREFIX(init)(LIST* lst, size_t sz);

LIST* PREFIX(new)(size_t sz);

LIST* PREFIX(new_init)(size_t sz, size_t count, ...);

void PREFIX(dealloc)(LIST* v);

void PREFIX(free)(LIST* v);

void PREFIX(reset)(LIST* lst);

int PREFIX(_realloc_array)(LIST* lst, size_t sz);

int PREFIX(makeroom)(LIST* lst, size_t sz);

int PREFIX(append)(LIST* lst, VALUE_T x);

VALUE_T PREFIX(poplast)(LIST* lst);

int PREFIX(insert)(LIST* lst, size_t i, VALUE_T x);

VALUE_T PREFIX(delete)(LIST* lst, size_t i);

VALUE_T PREFIX(fastdelete)(LIST* lst, size_t i);

int PREFIX(extend)(LIST* dst, const LIST* src);

int PREFIX(copy)(LIST* dst, const LIST* src);

LIST* PREFIX(new_copy)(const LIST* lst);

int PREFIX(reverse)(LIST* dst, const LIST* src);

/* eof */
