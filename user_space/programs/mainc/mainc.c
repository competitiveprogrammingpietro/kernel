#include "stdlib.h"
#include "string.h"

int main(char *in)
{
    print("\nHi mom look at the fun I am having\n");
    printf("My age is %i and I should be doing something different with my life really\n", 42);
    char *ptr = malloc(20);
    strcpy(ptr, "\nAddress allocated:\n");
    print(ptr);
    free(ptr);
    print("\nPress 'h' to exit");
    
    // ptr[2] = 'a'; // This causes an unhandled page fault
    while (1)
    {
        print(in);
        /*
        int c = getkey();
        if (c != 0)
        {
            putchar(c);
        }
        
        if (c == 'h' || c == 'H')
        {
            break;
        }
        */
    }
    return 0;
}