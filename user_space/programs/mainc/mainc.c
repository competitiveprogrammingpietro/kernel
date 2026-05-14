#include "stdlib.h"

int main(int argc, char **argv)
{
    print("Hi mom look at the fun I am having\n");
    print(atoi(1024));
    print("\n");
    putchar((int)'a');
    // printf("My age is %i\n", 42);
    char buffer[1024];
    printf("Readline....\n");
    readline(buffer, 1024, 1);
    print(buffer);
    print("\n");
    void *ptr = malloc(10);
    free(ptr);
    while (1)
    {
        int c = getkey();
        if (c != 0)
        {
            putchar(c);
        }
    }
    return 0;
}