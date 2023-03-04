
> Ref. [Learning Linux Binary Analys]

## Tools

### Objdump from GNU binutils

* View all data/code in every section of an ELF file:
objdump -D <elf_object>

* View only program code in an ELF file:
objdump -d <elf_object>

* View all symbols:
objdump -tT <elf_object>

### Objcopy from GNU binutils

```bash
# To copy the .data section from an ELF object to a file, use this line:
objcopy –only-section=.data <infile> <outfile>
```

### strace


System call trace (strace) is a tool that is based on the `ptrace(2)` system call, and it
utilizes the `PTRACE_SYSCALL` request in a loop to show information about the system
call (also known as syscall`s ) activity in a running program as well as signals that
are caught during execution.


### ltrace

library trace (ltrace) is another neat little tool, and it is very similar to strace . It
works similarly, but it actually parses the shared library-linking information of a
program and prints the library functions being used.

You may see system calls in addition to library function calls with the -S flag. The
ltrace command is designed to give more granular information, since it parses the
dynamic segment of the executable and prints actual symbols/functions from shared
and static libraries:

```
ltrace <program> -o program.out
```

### ftrace
It is similar to ltrace , but it also
shows calls to functions within the binary itself. There was no other tool I could find
publicly available that could do this in Linux, so I decided to code one. This tool can
be found at https://github.com/elfmaster/ftrace . A demonstration of this tool
is given in the next chapter.

### readelf

 - To retrieve a section header table:
    `readelf -S <object>`
 - To retrieve a symbol table:
    `readelf -s <object>`
 - To retrieve a program header table:
    `readelf -l <object>`
 - To retrieve the ELF file header data:
    `readelf -e <object>`
 - To retrieve relocation entries:
    `readelf -r <object>`
 - To retrieve a dynamic segment:
    `readelf -d <object>`


