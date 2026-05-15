#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>

void print(const char *str);
int getkey();
int getkey_blocking();
void *malloc(size_t size);
void free(void *ptr);
char *atoi(int n);
int putchar(int c);
int printf(const char *fmt, ...);
void readline(char *out, int n, int echo);
void exec(char * filename);

#endif