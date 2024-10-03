#ifndef _INCLUDED_ALLOCARRAY
#define _INCLUDED_ALLOCARRAY
#include <stdlib.h> /* imports *alloc() and free() declarations */
#include <assert.h> /* imports assert() macro */

/* allocate an array with n members of type typ */
#define ALLOC_ARRAY(ptr, typ, n) \
    { ptr = (typ *) malloc((n) * sizeof(typ)); assert(ptr); }

/* allocate an array with n members of type typ, initialized to zero */
#define CALLOC_ARRAY(ptr, typ, n) \
    { ptr = (typ *) calloc((n), sizeof(typ)); assert(ptr); }

/* free malloc'd memory w/pointer safeguard */
#define FREE_ARRAY(ptr) \
    { assert(ptr); free((void *) (ptr)); (ptr) = NULL; }

/* resize a previously-allocated array to have n members of type typ,
 * preserving contents */
#define REALLOC_ARRAY(ptr, typ, n) \
    { ptr = (typ *) realloc((void *) ptr, (n) * sizeof(typ)); assert(ptr); }

#endif /* _INCLUDED_ALLOCARRAY */
