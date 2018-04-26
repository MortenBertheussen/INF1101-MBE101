#ifndef COMPARE_H
#define COMPARE_H
#include <inttypes.h>
#include <string.h>

static inline void swap(void *a, void *b);
typedef int8_t (*cmpfunc_t)(void *, void *);
int8_t compare_strings(void *a, void *b);
int8_t numcmp(void *a, void *b);
#endif /*COMPARE_H*/
