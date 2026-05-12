#include "stdlib.h"

int main(int argc, char **argv)
{
    print("Hi mom look at the fun I am having\n");
    print(atoi(1024));
    print("\n");
    putchar((int)'a');
    printf("My age is %i\n", 42);
     void *ptr = malloc(10);
    free(ptr);
    while (1)
    {
        char buffer[2] = {.0};
        buffer[0] = getkey();
        if (getkey() != 0)
        {
            print("\n");
            printf("Key pressed %i\n", (int)buffer[0]);
            print(buffer);
            print("\n");
        }
    }
    return 0;
}