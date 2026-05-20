#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>

#define MAX_PROGRAM_ARG_LENGTH 512

// Linked list for program arguments
struct cmd_arg
{
    char arg[MAX_PROGRAM_ARG_LENGTH];
    struct cmd_arg *next;
};

void print(const char *str);
int getkey();
int getkey_blocking();
void *malloc(size_t size);
void free(void *ptr);
char *atoi(int n);
int putchar(int c);
int printf(const char *fmt, ...);
void readline(char *out, int n, int echo);
void exec(char *filename);
struct cmd_arg *parse_command(const char *command);
#endif