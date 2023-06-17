
## X86-64 Registers

> http://6.s081.scripts.mit.edu/sp18/x86-64-architecture-guide.html

| Register | Purpose                                | Saved across calls |
|----------|----------------------------------------|--------------------|
| `%rax`   | temp register; return value            | No                 |
| %rbx     | callee-saved                           | Yes                |
| %rcx     | used to pass 4th argument to functions | No                 |
| %rdx     | used to pass 3rd argument to functions | No                 |
| `%rsp`   | stack pointer                          | Yes                |
| `%rbp`   | callee-saved; base pointer             | Yes                |
| %rsi     | used to pass 2nd argument to functions | No                 |
| %rdi     | used to pass 1st argument to functions | No                 |
| %r8      | used to pass 5th argument to functions | No                 |
| %r9      | used to pass 6th argument to functions | No                 |
| %r10-r11 | temporary                              | No                 |
| %r12-r15 | callee-saved registers                 | Yes                |

>  https://cs61.seas.harvard.edu/site/2021/Asm/

| Full register (bits 0-63)      | 32-bit (bits 0–31) | 16-bit (bits 0–15) | 8-bit low (bits 0–7) | 8-bit high (bits 8–15) | Use in [calling convention](https://cs61.seas.harvard.edu/site/2021/Asm/#Calling-convention) | [Callee-saved?](https://cs61.seas.harvard.edu/site/2021/Asm/#Callee-saved) |
| ------------------------------ | ------------------ | ------------------ | -------------------- | ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| **General-purpose registers:** |                    |                    |                      |                        |                                                              |                                                              |
| **%rax**                       | %eax               | %ax                | %al                  | %ah                    | Return value (accumulator)                                   | No                                                           |
| **%rbx**                       | %ebx               | %bx                | %bl                  | %bh                    | –                                                            | **Yes**                                                      |
| **%rcx**                       | %ecx               | %cx                | %cl                  | %ch                    | 4th function parameter                                       | No                                                           |
| **%rdx**                       | %edx               | %dx                | %dl                  | %dh                    | 3rd function parameter Second return register (for 9–16 byte return values) | No                                                           |
| **%rsi**                       | %esi               | %si                | %sil                 | –                      | 2nd function parameter                                       | No                                                           |
| **%rdi**                       | %edi               | %di                | %dil                 | –                      | 1st function parameter                                       | No                                                           |
| **%r8**                        | %r8d               | %r8w               | %r8b                 | –                      | 5th function argument                                        | No                                                           |
| **%r9**                        | %r9d               | %r9w               | %r9b                 | –                      | 6th function argument                                        | No                                                           |
| **%r10**                       | %r10d              | %r10w              | %r10b                | –                      | –                                                            | No                                                           |
| **%r11**                       | %r11d              | %r11w              | %r11b                | –                      | –                                                            | No                                                           |
| **%r12**                       | %r12d              | %r12w              | %r12b                | –                      | –                                                            | **Yes**                                                      |
| **%r13**                       | %r13d              | %r13w              | %r13b                | –                      | –                                                            | **Yes**                                                      |
| **%r14**                       | %r14d              | %r14w              | %r14b                | –                      | –                                                            | **Yes**                                                      |
| **%r15**                       | %r15d              | %r15w              | %r15b                | –                      | –                                                            | **Yes**                                                      |
| **Special-purpose registers:** |                    |                    |                      |                        |                                                              |                                                              |
| **%rsp**                       | %esp               | %sp                | %spl                 | –                      | Stack pointer                                                | **Yes**                                                      |
| **%rbp**                       | %ebp               | %bp                | %bpl                 | –                      | Base pointer (general-purpose in many compiler modes)<br /><br />The %rbp register has a special purpose: it points to the bottom of the current function’s stack frame, and local variables are often accessed relative to its value. However, when optimization is on, the compiler may determine that all local variables can be stored in registers. This frees up %rbp for use as another general-purpose register. | **Yes**                                                      |
| **%rip**                       | %eip               | %ip                | –                    | –                      | Instruction pointer (Program counter; called $pc in GDB)     | *                                                            |
| **%rflags**                    | %eflags            | %flags             | –                    | –                      | Flags and condition codes                                    | No                                                           |



## Instruction

The basic kinds of assembly instructions are:

1. **Arithmetic.** These instructions perform computation on values, typically values stored in registers. Most have zero or one _source operands_ and one _source/destination operand_. The source operand is listed first in the _instruction_, but the source/destination operand comes first in the _computation_ (this matters for non-commutative operators like subtraction). For example, the instruction `addq %rax, %rbx` performs the computation `%rbx := %rbx + %rax`.
   
2. **Data movement.** These instructions move data between registers and memory. Almost all have one source operand and one destination operand; the source operand comes first.
   
3. **Control flow.** Normally the CPU executes instructions in sequence. Control flow instructions change the instruction pointer in other ways. There are unconditional branches (the instruction pointer is set to a new value), conditional branches (the instruction pointer is set to a new value if a condition is true), and function call and return instructions.

Abbreviation convention :

- b (byte) = 8 bits
- w (word) = 16 bits
- l (long) = 32 bits
- q (quad) = 64 bits

For instance, 

* `movzbl` moves an 8-bit quantity (a **b**yte) into a 32-bit register (a **l**ongword) with **z**ero extension; 

* `movslq` moves a 32-bit quantity (**l**ongword) into a 64-bit register (**q**uadword) with **s**ign extension.

### Instruction syntax

(We use the “AT&T syntax” for x86-64 assembly. For the “Intel syntax,” which you can find in online documentation from Intel, see the Aside in CS:APP3e §3.3, p177, or [Wikipedia](https://en.wikipedia.org/wiki/X86_assembly_language), or other online resources. AT&T syntax is distinguished by several features, but especially by the use of percent signs for registers. Unlike AT&T syntax, Intel syntax puts destination registers **before** source registers.)

Some instructions appear to combine arithmetic and data movement. For example, given the C code `int* ip; ... ++(*ip);` the compiler might generate `incl (%rax)` rather than `movl (%rax), %ebx; incl %ebx; movl %ebx, (%rax)`. However, the processor actually divides these complex instructions into tiny, simpler, invisible instructions called [microcode](https://en.wikipedia.org/wiki/Microcode), because the simpler instructions can be made to execute faster. The complex `incl` instruction actually runs in three phases: data movement, then arithmetic, then data movement. **This matters when we introduce parallelism**.

> https://notes.shichao.io/asm/

Instructions that take 2 operands. Notice how the format of the instruction is different for different assemblers.

```asm
Instr src, dest    # GAS Syntax
Instr dest, src    ; Intel syntax
```

Instructions that take 3 operands. Notice how the format of the instruction is different for different assemblers.

```asm
Instr aux, src, dest   # GAS Syntax
Instr dest, src, aux   ; Intel syntax
```

### Data Transfer Instructions

#### Move: `mov`

```asm
mov src, dest  # GAS Syntax
mov dest, src  ; Intel Syntax
```



## Directives

Assembly generated by a compiler contains instructions as well as *labels* and *directives*. Labels look like `labelname:` or `labelnumber:`; directives look like `.directivename arguments`. Labels are markers in the generated assembly, used to compute addresses. We usually see them used in control flow instructions, as in `jmp L3` (“jump to L3”). Directives are instructions to the assembler; for instance, the `.globl L` instruction says “label `L` is globally visible in the executable”, `.align` sets the alignment of the following data, `.long` puts a number in the output, and `.text` and `.data` define the current segment.

We also frequently look at assembly that is *disassembled* from executable instructions by GDB, `objdump -d`, or `objdump -S`. This output looks different from compiler-generated assembly: in disassembled instructions, there are no intermediate labels or directives. This is because the labels and directives disappear during the process of generating executable instructions.

For instance, here is some compiler-generated assembly:

```c
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
```

```asm
(gdb) disassemble /m myfunc
Dump of assembler code for function myfunc:
warning: Source file is more recent than executable.
5       int myfunc(struct Small small) {
   0x0000000000001149 <+0>:     endbr64 
   0x000000000000114d <+4>:     push   %rbp
   0x000000000000114e <+5>:     mov    %rsp,%rbp
   0x0000000000001151 <+8>:     sub    $0x10,%rsp
   0x0000000000001155 <+12>:    mov    %di,-0x2(%rbp)

6           if( small.field1 == '1' ) {
   0x0000000000001159 <+16>:    movzbl -0x2(%rbp),%eax
   0x000000000000115d <+20>:    cmp    $0x31,%al
   0x000000000000115f <+22>:    jne    0x1178 <myfunc+47>

7               small.field1 = 'a';
   0x0000000000001161 <+24>:    movb   $0x61,-0x2(%rbp)

8               printf("Hello, World a");
   0x0000000000001165 <+28>:    lea    0xe98(%rip),%rdi        # 0x2004
   0x000000000000116c <+35>:    mov    $0x0,%eax
   0x0000000000001171 <+40>:    callq  0x1050 <printf@plt>
   0x0000000000001176 <+45>:    jmp    0x118d <myfunc+68>

9           } else {
10              small.field1 = 'b';
   0x0000000000001178 <+47>:    movb   $0x62,-0x2(%rbp)

11              printf("Hello, World b");
   0x000000000000117c <+51>:    lea    0xe90(%rip),%rdi        # 0x2013
   0x0000000000001183 <+58>:    mov    $0x0,%eax
   0x0000000000001188 <+63>:    callq  0x1050 <printf@plt>

12          }
13
14          return small.field1 + 2 * small.field2;
   0x000000000000118d <+68>:    movzbl -0x2(%rbp),%eax
   0x0000000000001191 <+72>:    movsbl %al,%eax
   0x0000000000001194 <+75>:    movzbl -0x1(%rbp),%edx
   0x0000000000001198 <+79>:    movsbl %dl,%edx
   0x000000000000119b <+82>:    add    %edx,%edx
   0x000000000000119d <+84>:    add    %edx,%eax

15      }
   0x000000000000119f <+86>:    leaveq 
   0x00000000000011a0 <+87>:    retq   
```

## Address modes

| Type      | Example syntax        | Value used                                                   |
| --------- | --------------------- | ------------------------------------------------------------ |
| Register  | `%rbp`                | Contents of `%rbp`                                           |
| Immediate | `$0x4`                | 0x4                                                          |
| Memory    | `0x4`                 | Value stored at address 0x4                                  |
|           | `symbol_name`         | Value stored in global `symbol_name` (the compiler resolves the symbol name to an address when creating the executable) |
|           | `symbol_name(%rip)`   | [`%rip`-relative addressing](https://cs61.seas.harvard.edu/site/2021/Asm/#rip-relative-addressing) for global |
|           | `symbol_name+4(%rip)` | Simple arithmetic on symbols are allowed (the compiler resolves the arithmetic when creating the executable) |
|           | `(%rax)`              | Value stored at address in `%rax`                            |
|           | `0x4(%rax)`           | Value stored at address `%rax + 4`                           |
|           | `(%rax,%rbx)`         | Value stored at address `%rax + %rbx`                        |
|           | `(%rax,%rbx,4)`       | Value stored at address `%rax + %rbx*4`                      |
|           | `0x18(%rax,%rbx,4)`   | Value stored at address `%rax + 0x18 + %rbx*4`               |



## `%rip`-relative addressing

x86-64 code often refers to globals using **%rip-relative** addressing: a global variable named `a` is referenced as `a(%rip)` rather than `a`.

## Example

> https://cs61.seas.harvard.edu/site/2021/Kernel/

Here’s a fundamental attack on fair sharing of processor time. It’s the worst attack in the world:

```c++
int main() {
    while (true) {
    }
}
```

An infinite loop. Compiled to x86-64 instructions, this might be

```asm
00000000000005fa <main>:
 5fa:   55                      push   %rbp
 5fb:   48 89 e5                mov    %rsp,%rbp
 5fe:   eb fe                   jmp    5fe <main+0x4>
```

The critical instruction is `jmp 5fe`, represented in bytes as `eb fe`, which spins the processor in a tight loop forever.

> **Aside.** Why is this loop represented as `0xeb 0xfe`? An instruction consists of an opcode (e.g., “push”, “mov”, “pop”) and some operands (e.g., “%rbp”, “5fe”). Here, the `0xeb` part is the opcode. This opcode means “unconditional branch (`jmp`) by a relative one-byte offset”: when the instruction is executed, the `%rip` register will be modified by adding to it the signed offset stored as an operand. Here, that operand is `0xfe`, which, considered as a signed 8-bit number, is -2. Remember that when an instruction executes, the initial value of `%rip` is always the address of the _next_ instruction (because the processor must read the entire current instruction before executing it). Thus, adding -2 to `%rip` will reset `%rip` back to the start of the `jmp`.



# Links

https://cs61.seas.harvard.edu/site/2021/Asm/

https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html

https://notes.shichao.io/asm/
