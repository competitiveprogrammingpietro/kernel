#include "stdlib.h"

int main(int argc, char **argv)
{
    print("Hi mom look at the fun I am having\n");
    void *ptr = malloc(10);
    free(ptr);
    while (1)
    {
        if (getkey() != 0)
        {
            print("key pressed\n");
        }
    }
    return 0;
}