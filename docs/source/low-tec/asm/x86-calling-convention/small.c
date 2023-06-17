#include <stdio.h>

struct Small { char field1; char field2; };

int myfunc(struct Small small) {
    if( small.field1 == '1' ) {
        small.field1 = 'a';
        printf("Hello, World a");
    } else {
        small.field1 = 'b';
        printf("Hello, World b");
    }

    return small.field1 + 2 * small.field2;
}

int main(int argc, char *argv[])
{
    struct Small mysmall;
    myfunc(mysmall);
}

/**
gcc -fno-omit-frame-pointer -S ./small.c

gcc -fno-omit-frame-pointer  ./small.c -g 

objdump -d ./a.out

objdump -S ./a.out

gdb --tui ./a.out
(gdb) b main
(gdb) r
(gdb) layout split
// ctrl-x 2

disassemble /m

 * */