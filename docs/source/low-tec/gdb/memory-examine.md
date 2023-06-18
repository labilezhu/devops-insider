
```
(gdb) info proc mappings 
process 47983
Mapped address spaces:

          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /home/labile/devops-insider/docs/source/low-tec/asm/x86-calling-convention/a.out
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /home/labile/devops-insider/docs/source/low-tec/asm/x86-calling-convention/a.out
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /home/labile/devops-insider/docs/source/low-tec/asm/x86-calling-convention/a.out
      0x555555557000     0x555555558000     0x1000     0x2000  r--p   /home/labile/devops-insider/docs/source/low-tec/asm/x86-calling-convention/a.out
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   /home/labile/devops-insider/docs/source/low-tec/asm/x86-calling-convention/a.out
      0x7ffff7d73000     0x7ffff7d76000     0x3000        0x0  rw-p   
      0x7ffff7d76000     0x7ffff7d9e000    0x28000        0x0  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7d9e000     0x7ffff7f33000   0x195000    0x28000  r-xp   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7f33000     0x7ffff7f8b000    0x58000   0x1bd000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7f8b000     0x7ffff7f8f000     0x4000   0x214000  r--p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7f8f000     0x7ffff7f91000     0x2000   0x218000  rw-p   /usr/lib/x86_64-linux-gnu/libc.so.6
      0x7ffff7f91000     0x7ffff7f9e000     0xd000        0x0  rw-p   
      0x7ffff7fbb000     0x7ffff7fbd000     0x2000        0x0  rw-p   
      0x7ffff7fbd000     0x7ffff7fc1000     0x4000        0x0  r--p   [vvar]
      0x7ffff7fc1000     0x7ffff7fc3000     0x2000        0x0  r-xp   [vdso]
      0x7ffff7fc3000     0x7ffff7fc5000     0x2000        0x0  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fc5000     0x7ffff7fef000    0x2a000     0x2000  r-xp   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7fef000     0x7ffff7ffa000     0xb000    0x2c000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffb000     0x7ffff7ffd000     0x2000    0x37000  r--p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffff7ffd000     0x7ffff7fff000     0x2000    0x39000  rw-p   /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
      0x7ffffffde000     0x7ffffffff000    0x21000        0x0  rw-p   [stack]
  0xffffffffff600000 0xffffffffff601000     0x1000        0x0  --xp   [vsyscall]

```


```

```