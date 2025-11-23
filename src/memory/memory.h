#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>
void* memset(void*, int, size_t);
int memcmp(void* s1, void* s2, int count);
void* memcpy(void* dst, void* src, int len);
#endif
