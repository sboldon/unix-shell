#ifndef _ALLOC_TEST_H
#define _ALLOC_TEST_H

#include <stdlib.h>

void set_fail_after(int num);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

// #define malloc(x) xmalloc(x)
// #define realloc(x, y) xrealloc(x, y)

#endif
