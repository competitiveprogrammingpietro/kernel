#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("\nHi mom look at the fun I am having\n");
    print(atoi(1024));
    print("\n");
    putchar((int)'a');
    // printf("My age is %i\n", 42);
    char *ptr = malloc(20);
    strcpy(ptr, "\nIAMMALLOCMEM\n");
    print(ptr);
    free(ptr);
    ptr[2] = 'a'; // This causes an unhandled page fault
    print("AFTER FREE");
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