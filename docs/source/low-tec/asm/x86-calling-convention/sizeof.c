#include <inttypes.h>
#include <stdio.h>

int main()
{
    printf("    short int: %zd\n", sizeof(short int));
    printf("          int: %zd\n", sizeof(int));
    printf("         long: %zd\n", sizeof(long));
    printf("     long int: %zd\n", sizeof(long int));
    printf("long long int: %zd\n", sizeof(long long int));
    printf("       size_t: %zd\n", sizeof(size_t));
    printf("        void*: %zd\n\n", sizeof(void *));

    printf("PRIu32 usage (see source): %" PRIu32 "\n", (uint32_t)42);
    return 0;
}

// gcc ./sizeof.c && ./a.out

/**
    short int: 2
          int: 4
         long: 8
     long int: 8
long long int: 8
       size_t: 8
        void*: 8

PRIu32 usage (see source): 42
**/

