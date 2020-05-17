#include <stdio.h>
#include <stdlib.h>
#include "alloc_test.h"

static int fail_after = 0;
static int num_allocs = 0;

void set_fail_after(int num) {
  fail_after = num;
}

void *xmalloc(size_t size) {
  if (fail_after > 0 && num_allocs++ >= fail_after) {
    fputs("Out of memory: allocation failed\n", stderr);
    return NULL;
  }
  return malloc(size);
}

void *xrealloc(void *ptr, size_t size) {
  if (fail_after > 0 && num_allocs++ >= fail_after) {
    fputs("Out of memory: allocation failed\n", stderr);
    return NULL;
  }
  return realloc(ptr, size);
}
