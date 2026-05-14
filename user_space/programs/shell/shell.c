#include "shell.h"
#include "stdlib.h"

int main(int argc, char **argv)
{
    char line[1024] = {.0};

    print("Welcome to pedr0 OS shell v1.0.0\n");
    while (1)
    {
        print("$>");
        readline(line, sizeof(line), 1);
        print("\n");
        print(line);
        print("\n");
    }
    return 0;
}