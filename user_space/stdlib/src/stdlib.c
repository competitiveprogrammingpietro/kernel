#include "stdlib.h"

extern void asm_print();
extern int asm_getkey();
extern void *asm_malloc();
extern void asm_free();

void print(const char *str)
{
    asm_print(str);
}

int getkey()
{
    return asm_getkey();
}

void *malloc(size_t size)
{
    return asm_malloc(size);
}

void free(void *ptr)
{
    asm_free(ptr);
}