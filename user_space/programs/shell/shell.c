#include "shell.h"
#include "stdlib.h"

int main(int argc, char **argv)
{
    char line[1024] = {.0};
    char argument[32] = "abcde";
    print("Welcome to pedr0 OS shell v1.0.0\n");
    while (1)
    {
        print("$>");
        readline(line, sizeof(line), 1);
        int res = exec(line, argument);
        if (res < 0)
       {
        print("Error when executing the command\n");
       }
        print("\n");
        print(line);
        print("\n");
    }
    return 0;
}