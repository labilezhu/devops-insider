#include <stdio.h>
#include <stdlib.h>

const char *p = "Hello World\n";

void myfunc()
{
    printf("%s", p);
}

int main()
{
    //asm("int $3");
    printf("%s", p);
    myfunc();

    printf("Please enter a line:\n");
    char *line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
    lineSize = getline(&line, &len, stdin);
    printf("You entered %s, which has %zu chars.\n", line, lineSize - 1);
    free(line);

    return 0;
}
// gcc -g breakpoints.c
