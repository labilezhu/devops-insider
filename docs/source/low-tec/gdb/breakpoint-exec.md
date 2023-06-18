
```
(gdb) disassemble myfunc
Dump of assembler code for function myfunc:
   0x0000555555555149 <+0>:	endbr64 
   0x000055555555514d <+4>:	push   %rbp
   0x000055555555514e <+5>:	mov    %rsp,%rbp
   0x0000555555555151 <+8>:	mov    0x2eb8(%rip),%rax        # 0x555555558010 <p>
   0x0000555555555158 <+15>:	mov    %rax,%rsi
   0x000055555555515b <+18>:	lea    0xeaf(%rip),%rdi        # 0x555555556011
   0x0000555555555162 <+25>:	mov    $0x0,%eax
   0x0000555555555167 <+30>:	callq  0x555555555050 <printf@plt>
   0x000055555555516c <+35>:	nop
   0x000055555555516d <+36>:	pop    %rbp
   0x000055555555516e <+37>:	retq   
End of assembler dump.
(gdb) info address myfunc 
Symbol "myfunc" is a function at address 0x555555555149.

```

```
cd /proc/`pgrep a.out`

pmap `pgrep a.out`

labile@labile-T30 ➜ 11296 $ pmap -X `pgrep a.out`
11296:   /nfs/shareset/home/blog/content/zh/notes/low-tec/gdb/a.out
         Address Perm   Offset Device    Inode Size Rss Pss Referenced Anonymous LazyFree ShmemPmdMapped FilePmdMapped Shared_Hugetlb Private_Hugetlb Swap SwapPss Locked THPeligible Mapping
    555555554000 r--p 00000000  08:12 15606718    4   4   4          4         4        0              0             0              0               0    0       0      0           0 a.out
    555555555000 r-xp 00001000  08:12 15606718    4   4   4          4         4        0              0             0              0               0    0       0      0           0 a.out
    555555556000 r--p 00002000  08:12 15606718    4   0   0          0         0        0              0             0              0               0    0       0      0           0 a.out
    555555557000 r--p 00002000  08:12 15606718    4   4   4          4         4        0              0             0              0               0    0       0      0           0 a.out
    555555558000 rw-p 00003000  08:12 15606718    4   4   4          4         4        0              0             0              0               0    0       0      0           0 a.out
    7ffff7dbf000 r--p 00000000  08:12  5506644  148 140   0        140         0        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7de4000 r-xp 00025000  08:12  5506644 1504 428  10        428         8        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7f5c000 r--p 0019d000  08:12  5506644  296  64   0         64         0        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7fa6000 ---p 001e7000  08:12  5506644    4   0   0          0         0        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7fa7000 r--p 001e7000  08:12  5506644   12  12  12         12        12        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7faa000 rw-p 001ea000  08:12  5506644   12  12  12         12        12        0              0             0              0               0    0       0      0           0 libc-2.31.so
    7ffff7fad000 rw-p 00000000  00:00        0   24  20  20         20        20        0              0             0              0               0    0       0      0           0 
    7ffff7fcb000 r--p 00000000  00:00        0   12   0   0          0         0        0              0             0              0               0    0       0      0           0 [vvar]
    7ffff7fce000 r-xp 00000000  00:00        0    4   4   4          4         4        0              0             0              0               0    0       0      0           0 [vdso]
    7ffff7fcf000 r--p 00000000  08:12  5506640    4   4   0          4         0        0              0             0              0               0    0       0      0           0 ld-2.31.so
    7ffff7fd0000 r-xp 00001000  08:12  5506640  140 140  24        140        24        0              0             0              0               0    0       0      0           0 ld-2.31.so
    7ffff7ff3000 r--p 00024000  08:12  5506640   32  32   0         32         0        0              0             0              0               0    0       0      0           0 ld-2.31.so
    7ffff7ffc000 r--p 0002c000  08:12  5506640    4   4   4          4         4        0              0             0              0               0    0       0      0           0 ld-2.31.so
    7ffff7ffd000 rw-p 0002d000  08:12  5506640    4   4   4          4         4        0              0             0              0               0    0       0      0           0 ld-2.31.so
    7ffff7ffe000 rw-p 00000000  00:00        0    4   4   4          4         4        0              0             0              0               0    0       0      0           0 
    7ffffffde000 rw-p 00000000  00:00        0  132  12  12         12        12        0              0             0              0               0    0       0      0           0 [stack]
ffffffffff600000 --xp 00000000  00:00        0    4   0   0          0         0        0              0             0              0               0    0       0      0           0 [vsyscall]
                                               ==== === === ========== ========= ======== ============== ============= ============== =============== ==== ======= ====== =========== 
                                               2360 896 122        896       120        0              0             0              0               0    0       0      0           0 KB 

```

```
b *main 与 b main 是不同的。前者在法函数入口点。
```