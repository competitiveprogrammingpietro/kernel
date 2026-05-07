#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>

void print(const char *str);
int getkey();
void *malloc(size_t size);
void free(void *ptr);
#endif