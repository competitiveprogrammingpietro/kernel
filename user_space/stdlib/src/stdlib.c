#include "stdlib.h"
#include <stdarg.h>

extern void asm_print();
extern int asm_getkey();
extern void *asm_malloc();
extern void asm_free();
extern void asm_putchar(int c);
extern void asm_exec(char *);

void print(const char *str)
{
    asm_print(str);
}

int getkey()
{
    return asm_getkey();
}

int getkey_blocking()
{
    // This function spins until a key is read
    int c = 0;
    while ((c = asm_getkey()) == 0)
        ;
    return c;
}

int putchar(int c)
{
    asm_putchar(c);
    return 0;
}

void *malloc(size_t size)
{
    return asm_malloc(size);
}

void free(void *ptr)
{
    asm_free(ptr);
}

// This printf implementantion allows only for two types of data:
// 1. Strings
// 2. Integers
int printf(const char *fmt, ...)
{
    va_list ap;
    const char *p;
    char *sval;
    int ival;

    va_start(ap, fmt);
    for (p = fmt; *p; p++)
    {
        if (*p != '%')
        {
            putchar(*p);
            continue;
        }

        switch (*++p)
        {
        case 'i':
            ival = va_arg(ap, int);
            print(atoi(ival));
            break;

        case 's':
            sval = va_arg(ap, char *);
            print(sval);
            break;

        default:
            return -1; // Not allowed
        }
    }
    va_end(ap);
    return 0;
}

// This first implemenation returns a ptr not to be shared or written on, the
// real use case we're addressing is the printing of a number, that is that.
char *atoi(int n)
{
    static char text[12]; // 32 bits are 9 digits numbers, plus sign and NULL makes 12

    int index = 11;
    text[11] = 0;

    char neg = 1;
    if (n >= 0)
    {
        neg = 0;
    }

    while (n)
    {
        text[--index] = '0' + (n % 10);
        n /= 10;
    }

    if (index == 11)
        text[--index] = '0';

    if (neg)
        text[--index] = '-';

    return &text[index];
}

void readline(char *out, int n, int echo)
{
    int i = 0;
    while (i < n - 1)
    {
        char c = (char)getkey_blocking();

        // Carriage return '\r', 13
        if (c == '\r')
        {
            break;
        }

        if (c == '\n')
        {
            break;
        }

        if (echo)
        {
            putchar(c);
        }

        // Backspace
        if (c == '\b' && i > 0)
        {
            i--;
            out[i] = 0x00;
            continue;
        }
        out[i] = c;
        i++;
    }

    // Add the null terminator
    out[i] = 0x00;
}

void exec(char *filename)
{
    asm_exec(filename);
}

struct cmd_arg *parse_command(const char *command)
{
    struct cmd_arg *root = 0;

    // Copy of the command
    char scommand[1024];
    strncpy(scommand, command, sizeof(scommand));

    char *token = strtok(scommand, " ");
    if (!token)
    {
        return root;
    }

    root = malloc(sizeof(struct cmd_arg));
    if (!root)
    {
        return 0;
    }

    strncpy(root->arg, token, sizeof(root->arg));
    root->next = 0;

    struct cmd_arg *current = root;
    token = strtok(NULL, " ");
    while (token != 0)
    {
        struct cmd_arg *narg = peachos_malloc(sizeof(struct cmd_arg));
        if (!narg)
        {
            break;
        }

        strncpy(narg->arg, token, sizeof(narg->arg));
        narg->next = 0x00;
        current->next = narg;
        current = narg;
        token = strtok(NULL, " ");
    }
    return root;
}