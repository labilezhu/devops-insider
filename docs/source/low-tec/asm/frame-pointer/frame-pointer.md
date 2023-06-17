> Ref. [Learning Linux Binary Analys]

```
$ objdump -d test2
test2:
file format elf64-x86-64
Disassembly of section .text:
0000000000400144 <.text>:
400144: 55 push %rbp
400145: 48 89 e5 mov %rsp,%rbp
400148: 5d pop %rbp
400149: c3 retq 40014a: 55 push %rbp

40014b: 48 89 e5 mov %rsp,%rbp
40014e: e8 f1 ff ff ff callq 0x400144
400153: c9 leaveq 
400154: 5d pop 400155: c3 retq
```

The only thing to give us an idea where a new function starts is by examining
the `procedure prologue`, which is at the beginning of every function, unless ( gcc-fomit-frame-pointer ) has been used, in which case it becomes less obvious
to identify.