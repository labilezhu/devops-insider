#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }

    execl("/bin/ls", "ls", "-lh", "a" ,NULL);

    return 0;
}