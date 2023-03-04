
# ELF symbols

> Ref. [Learning Linux Binary Analys]


Symbols are a symbolic reference to some type of data or code such as a global
variable or function. For instance, the `printf()` function is going to have a symbol
entry that points to it in the dynamic symbol table `.dynsym` . In most shared libraries
and dynamically linked executables, there exist two symbol tables. In the `readelf -S` output shown previously, you can see two sections: `.dynsym` and `.symtab` .

- `.dynsym`
  - contains global symbols that reference symbols from an external
  source, such as libc functions like `printf` . `.dynsym` symbol table is necessary for the execution of dynamically linked executables.

- `.symtab`
  - will contain all of the symbols in `.dynsym` , **as well as the local symbols for the executable**, such as global variables, or local functions that you have defined in your code. So `.symtab` contains all of the symbols, whereas `.dynsym` contains just the dynamic/global symbols. `.symtab` symbol table exists only for debugging and linking purposes and is often stripped (removed) from production binaries to save space.

Let's take a look at what an ELF symbol entry looks like for 64-bit ELF files:

```c
typedef struct {
    uint32_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
    Elf64_Addr st_value;
    Uint64_t st_size;
} Elf64_Sym;
```

Symbol entries are contained within the `.symtab` and `.dynsym` sections, which is
why the sh_entsize (section header entry size) for those sections are equivalent
to sizeof(ElfN_Sym) .

- st_name
The st_name contains an offset into the symbol table's string table (located in either
.dynstr or .strtab ), where the name of the symbol is located, such as printf .
- st_value
  The st_value holds the value of the symbol (either an address or offset of
  its location).
  > http://sco.com/developers/gabi/latest/ch4.symtab.html#symbol_value
  >
  > Symbol table entries for different object file types have slightly different interpretations for the `st_value` member.
  >
  > - In relocatable files, `st_value` holds alignment constraints for a symbol whose section index is `SHN_COMMON`.
  >   
  > - In relocatable files, `st_value` holds a section offset for a defined symbol. `st_value` is an offset from the beginning of the section that `st_shndx` identifies.
  >   
  > - In executable and shared object files, `st_value` holds a virtual address. To make these files' symbols more useful for the dynamic linker, the section offset (file interpretation) gives way to a virtual address (memory interpretation) for which the section number is irrelevant.
- st_size
The st_size contains the size of the symbol, such as the size of a global function
ptr , which would be 4 bytes on a 32-bit system.

### st_info

#### Symbol types
We've got the following symbol types:
• STT_NOTYPE : The symbols type is undefined
• STT_FUNC : The symbol is associated with a function or other executable code
• STT_OBJECT : The symbol is associated with a data object
#### Symbol bindings
We've got the following symbol bindings:
- STB_LOCAL : Local symbols are not visible outside the object file containing
their definition, such as a function declared static.

- STB_GLOBAL : Global symbols are visible to all object files being combined.
One file's definition of a global symbol will satisfy another file's undefined
reference to the same symbol.

- STB_WEAK : Similar to global binding, but with less precedence, meaning that
the binding is weak and may be overridden by another symbol (with the
same name) that is not marked as STB_WEAK .



Let's look at the symbol table for the following source code:
```c
static inline void foochu()
{ /* Do nothing */ }

void func1()
{ /* Do nothing */ }

_start()
{
    func1();
    foochu();
}
```

```bash
ryan@alchemy:~$ readelf -s test | egrep 'foochu|func1'
7: 080480d8 5 FUNC LOCAL  DEFAULT 2 foochu
8: 080480dd 5 FUNC GLOBAL DEFAULT 2 func1
```

```c
#include <stdio.h>

int func1(int a, int b, int c
{
    printf("%d %d %d\n", a, b ,c);
}

int main(void)
{
    func1(1, 2, 3);
}
```

```
ryan@alchemy:~$ ftrace -s test
[+] Function tracing begins here:
PLT_call@0x400420:__libc_start_main()
LOCAL_call@0x4003e0:_init()
(RETURN VALUE) LOCAL_call@0x4003e0: _init() = 0
LOCAL_call@0x40052c:func1(0x1,0x2,0x3) // notice values passed
PLT_call@0x400410:printf("%d %d %d\n") // notice we see string value
1 2 3
(RETURN VALUE) PLT_call@0x400410: printf("%d %d %d\n") = 6
(RETURN VALUE) LOCAL_call@0x40052c: func1(0x1,0x2,0x3) = 6
LOCAL_call@0x400470:deregister_tm_clones()
(RETURN VALUE) LOCAL_call@0x400470: deregister_tm_clones() = 7
```

