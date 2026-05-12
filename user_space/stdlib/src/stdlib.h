#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>

void print(const char *str);
int getkey();
void *malloc(size_t size);
void free(void *ptr);
char *atoi(int n);
int putchar(int c);
int printf(const char *fmt, ...);
#endif