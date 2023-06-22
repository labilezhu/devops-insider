


```bash
gcc -g main-argv.c -o main-argv.exe


gdb -ex 'set disable-randomization on'  --args ./main-argv.exe a bc d



b *main
Breakpoint 2 at 0x1149: file main-argv.c, line 3.
b main
Breakpoint 2 at 0x55555555515c: file main-argv.c, line 4.

run

Breakpoint 1, main (argc=0, argv=0x0) at main-argv.c:3
3	int main(int argc, char *argv[]) {
```


```asm
(gdb) disassemble 
Dump of assembler code for function main:
=> 0x0000555555555149 <+0>:	endbr64 
   0x000055555555514d <+4>:	push   %rbp
   0x000055555555514e <+5>:	mov    %rsp,%rbp
   0x0000555555555151 <+8>:	sub    $0x20,%rsp
   0x0000555555555155 <+12>:	mov    %edi,-0x14(%rbp)
   0x0000555555555158 <+15>:	mov    %rsi,-0x20(%rbp)
   0x000055555555515c <+19>:	movl   $0x0,-0x4(%rbp)
   0x0000555555555163 <+26>:	jmp    0x555555555188 <main+63>
   0x0000555555555165 <+28>:	mov    -0x4(%rbp),%eax
   0x0000555555555168 <+31>:	cltq   
   0x000055555555516a <+33>:	lea    0x0(,%rax,8),%rdx
   0x0000555555555172 <+41>:	mov    -0x20(%rbp),%rax
   0x0000555555555176 <+45>:	add    %rdx,%rax
   0x0000555555555179 <+48>:	mov    (%rax),%rax
   0x000055555555517c <+51>:	mov    %rax,%rdi
   0x000055555555517f <+54>:	call   0x555555555050 <puts@plt>
   0x0000555555555184 <+59>:	addl   $0x1,-0x4(%rbp)
   0x0000555555555188 <+63>:	mov    -0x4(%rbp),%eax
   0x000055555555518b <+66>:	cmp    -0x14(%rbp),%eax
   0x000055555555518e <+69>:	jl     0x555555555165 <main+28>
   0x0000555555555190 <+71>:	mov    $0x0,%eax
   0x0000555555555195 <+76>:	leave  
   0x0000555555555196 <+77>:	ret  

(gdb) c
Continuing.

Breakpoint 2, main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:4
4	  for (int i = 0; i < argc; i++) {
(gdb) disassemble 
Dump of assembler code for function main:
   0x0000555555555149 <+0>:	endbr64 
   0x000055555555514d <+4>:	push   %rbp
   0x000055555555514e <+5>:	mov    %rsp,%rbp
   0x0000555555555151 <+8>:	sub    $0x20,%rsp
   0x0000555555555155 <+12>:	mov    %edi,-0x14(%rbp)
   0x0000555555555158 <+15>:	mov    %rsi,-0x20(%rbp)
=> 0x000055555555515c <+19>:	movl   $0x0,-0x4(%rbp)
   0x0000555555555163 <+26>:	jmp    0x555555555188 <main+63>
   0x0000555555555165 <+28>:	mov    -0x4(%rbp),%eax
   0x0000555555555168 <+31>:	cltq   
   0x000055555555516a <+33>:	lea    0x0(,%rax,8),%rdx
   0x0000555555555172 <+41>:	mov    -0x20(%rbp),%rax
   0x0000555555555176 <+45>:	add    %rdx,%rax
   0x0000555555555179 <+48>:	mov    (%rax),%rax
   0x000055555555517c <+51>:	mov    %rax,%rdi
   0x000055555555517f <+54>:	call   0x555555555050 <puts@plt>
   0x0000555555555184 <+59>:	addl   $0x1,-0x4(%rbp)
   0x0000555555555188 <+63>:	mov    -0x4(%rbp),%eax
   0x000055555555518b <+66>:	cmp    -0x14(%rbp),%eax
   0x000055555555518e <+69>:	jl     0x555555555165 <main+28>
   0x0000555555555190 <+71>:	mov    $0x0,%eax
   0x0000555555555195 <+76>:	leave  
   0x0000555555555196 <+77>:	ret    
End of assembler dump.


(gdb) bt
#0  main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:4

(gdb) p/x $rsi
$3 = 0x7fffffffdcc8

(gdb) p *((char**)$rsi)
$1 = 4

(gdb) p *((char**)$rsi)
$6 = 0x7fffffffe080 "/home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe"

set $a=((char**)$rsi)

(gdb) p $a
$10 = (char **) 0x7fffffffdcc8
(gdb) p $a+1
$11 = (char **) 0x7fffffffdcd0
(gdb) set $a1=$a+1
(gdb) p $a1
$12 = (char **) 0x7fffffffdcd0
(gdb) p *$a1
$13 = 0x7fffffffe0cc "a"

(gdb) p *((char**)($rsi+8))
$14 = 0x7fffffffe0cc "a"

(gdb) p *((char**)($rsi+(8*2)))
$15 = 0x7fffffffe0ce "bc"

(gdb) x/100bc *((char**)($rsi))
0x7fffffffe080:	47 '/'	104 'h'	111 'o'	109 'm'	101 'e'	47 '/'	108 'l'	97 'a'
0x7fffffffe088:	98 'b'	105 'i'	108 'l'	101 'e'	47 '/'	100 'd'	101 'e'	118 'v'
0x7fffffffe090:	111 'o'	112 'p'	115 's'	45 '-'	105 'i'	110 'n'	115 's'	105 'i'
0x7fffffffe098:	100 'd'	101 'e'	114 'r'	47 '/'	100 'd'	111 'o'	99 'c'	115 's'
0x7fffffffe0a0:	47 '/'	115 's'	111 'o'	117 'u'	114 'r'	99 'c'	101 'e'	47 '/'
0x7fffffffe0a8:	108 'l'	111 'o'	119 'w'	45 '-'	116 't'	101 'e'	99 'c'	47 '/'
0x7fffffffe0b0:	103 'g'	100 'd'	98 'b'	47 '/'	109 'm'	97 'a'	105 'i'	110 'n'
0x7fffffffe0b8:	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	47 '/'	109 'm'	97 'a'
0x7fffffffe0c0:	105 'i'	110 'n'	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	46 '.'
0x7fffffffe0c8:	101 'e'	120 'x'	101 'e'	0 '\000'	97 'a'	0 '\000'	98 'b'	99 'c'
0x7fffffffe0d0:	0 '\000'	100 'd'	0 '\000'


x/20xg $rsi

```

## argv array
```
(gdb) x/20xg $rsi
0x7fffffffdcc8:	0x00007fffffffe080	0x00007fffffffe0cc
0x7fffffffdcd8:	0x00007fffffffe0ce	0x00007fffffffe0d1
0x7fffffffdce8:	0x0000000000000000	0x00007fffffffe0d3

p (char*)0x00007fffffffe080


(gdb) x/20xg $rsi
0x7fffffffdcc8:	0x00007fffffffe080	0x00007fffffffe0cc
0x7fffffffdcd8:	0x00007fffffffe0ce	0x00007fffffffe0d1
0x7fffffffdce8:	0x0000000000000000	0x00007fffffffe0d3
0x7fffffffdcf8:	0x00007fffffffe115	0x00007fffffffe124
0x7fffffffdd08:	0x00007fffffffe135	0x00007fffffffe146
0x7fffffffdd18:	0x00007fffffffe15f	0x00007fffffffe173
0x7fffffffdd28:	0x00007fffffffe19c	0x00007fffffffe1ba
0x7fffffffdd38:	0x00007fffffffe1ee	0x00007fffffffe220
0x7fffffffdd48:	0x00007fffffffe27a	0x00007fffffffe29d
0x7fffffffdd58:	0x00007fffffffe2d1	0x00007fffffffe2dd
(gdb) p (char*)0x00007fffffffe080
$16 = 0x7fffffffe080 "/home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe"
(gdb) p (char*)0x00007fffffffe0cc
$17 = 0x7fffffffe0cc "a"
(gdb) p (char*)0x00007fffffffe0ce
$18 = 0x7fffffffe0ce "bc"
(gdb) p (char*)0x00007fffffffe0d1
$19 = 0x7fffffffe0d1 "d"

set {char [3]} 0x00007fffffffe0ce = "xy"


```


```
(gdb) bt -past-main on -past-entry on
#0  main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:4
#1  0x00007ffff7d9fd90 in __libc_start_call_main (main=main@entry=0x555555555149 <main>, argc=argc@entry=4, argv=argv@entry=0x7fffffffdcc8) at ../sysdeps/nptl/libc_start_call_main.h:58
#2  0x00007ffff7d9fe40 in __libc_start_main_impl (main=0x555555555149 <main>, argc=4, argv=0x7fffffffdcc8, init=<optimized out>, fini=<optimized out>, rtld_fini=<optimized out>, stack_end=0x7fffffffdcb8) at ../csu/libc-start.c:392
#3  0x0000555555555085 in _start ()

```


> [/proc/$pid/stat](https://man7.org/linux/man-pages/man5/proc.5.html)


```bash
pid=`pgrep main-argv.exe`&& a=(`cat /proc/$pid/stat`) && echo ${a[48]} 
140737488347264
```


```
(gdb) x/100bc 140737488347264
0x7fffffffe080:	47 '/'	104 'h'	111 'o'	109 'm'	101 'e'	47 '/'	108 'l'	97 'a'
0x7fffffffe088:	98 'b'	105 'i'	108 'l'	101 'e'	47 '/'	100 'd'	101 'e'	118 'v'
0x7fffffffe090:	111 'o'	112 'p'	115 's'	45 '-'	105 'i'	110 'n'	115 's'	105 'i'
0x7fffffffe098:	100 'd'	101 'e'	114 'r'	47 '/'	100 'd'	111 'o'	99 'c'	115 's'
0x7fffffffe0a0:	47 '/'	115 's'	111 'o'	117 'u'	114 'r'	99 'c'	101 'e'	47 '/'
0x7fffffffe0a8:	108 'l'	111 'o'	119 'w'	45 '-'	116 't'	101 'e'	99 'c'	47 '/'
0x7fffffffe0b0:	103 'g'	100 'd'	98 'b'	47 '/'	109 'm'	97 'a'	105 'i'	110 'n'
0x7fffffffe0b8:	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	47 '/'	109 'm'	97 'a'
0x7fffffffe0c0:	105 'i'	110 'n'	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	46 '.'
0x7fffffffe0c8:	101 'e'	120 'x'	101 'e'	0 '\000'	97 'a'	0 '\000'	120 'x'	121 'y'
0x7fffffffe0d0:	0 '\000'	100 'd'	0 '\000'	80 'P'	87 'W'	68 'D'	61 '='	47 '/'
0x7fffffffe0d8:	104 'h'	111 'o'	109 'm'	101 'e'	47 '/'	108 'l'	97 'a'	98 'b'
0x7fffffffe0e0:	105 'i'	108 'l'	101 'e'	47 '/'
```


```c
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
```


```log
labile@labile-T30 ➜ main-argv $ gdb -ex 'set disable-randomization on'  --args ./main-argv.exe a bc d
Reading symbols from ./main-argv.exe...
(gdb) b execl
Breakpoint 1 at 0x1070
(gdb) run
Starting program: /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe a bc d
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
/home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
a
bc
d

Breakpoint 1, __GI_execl (path=0x55555555600d "/bin/ls", arg=0x55555555600a "ls") at ./posix/execl.c:28
28	./posix/execl.c: No such file or directory.
(gdb) bt
#0  __GI_execl (path=0x55555555600d "/bin/ls", arg=0x55555555600a "ls") at ./posix/execl.c:28
#1  0x00005555555551e8 in main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:11
(gdb) catch syscall
Catchpoint 2 (any syscall)
(gdb) c
Continuing.

Catchpoint 2 (call to syscall execve), 0x00007ffff7e610fb in execve () at ../sysdeps/unix/syscall-template.S:120
120	../sysdeps/unix/syscall-template.S: No such file or directory.
(gdb) bt
#0  0x00007ffff7e610fb in execve () at ../sysdeps/unix/syscall-template.S:120
#1  0x00007ffff7e61615 in __GI_execl (path=<optimized out>, arg=0x55555555600a "ls") at ./posix/execl.c:56
#2  0x00005555555551e8 in main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:11

p *((char**)$rsi)
$9 = 0x55555555600a "ls"

(gdb) p *((char**)($rsi+8))
$11 = 0x555555556006 "-lh"

(gdb) p *((char**)$rdx)
$12 = 0x7fffffffe0d3 "PWD=/home/labile/devops-insider/docs/source/low-tec/gdb/main-argv"

(gdb) p *((char**)($rdx+8))
$13 = 0x7fffffffe115 "LANGUAGE=en_US"
```

## get argv of main()

```
labile@labile-T30 ➜ labile $ pid=`pgrep main-argv.exe`&& a=(`sudo cat /proc/$pid/stat`) && echo ${a[48]} 
140737488347264
```


```
(gdb) x/100bc 140737488347264
0x7fffffffe080:	47 '/'	104 'h'	111 'o'	109 'm'	101 'e'	47 '/'	108 'l'	97 'a'
0x7fffffffe088:	98 'b'	105 'i'	108 'l'	101 'e'	47 '/'	100 'd'	101 'e'	118 'v'
0x7fffffffe090:	111 'o'	112 'p'	115 's'	45 '-'	105 'i'	110 'n'	115 's'	105 'i'
0x7fffffffe098:	100 'd'	101 'e'	114 'r'	47 '/'	100 'd'	111 'o'	99 'c'	115 's'
0x7fffffffe0a0:	47 '/'	115 's'	111 'o'	117 'u'	114 'r'	99 'c'	101 'e'	47 '/'
0x7fffffffe0a8:	108 'l'	111 'o'	119 'w'	45 '-'	116 't'	101 'e'	99 'c'	47 '/'
0x7fffffffe0b0:	103 'g'	100 'd'	98 'b'	47 '/'	109 'm'	97 'a'	105 'i'	110 'n'
0x7fffffffe0b8:	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	47 '/'	109 'm'	97 'a'
0x7fffffffe0c0:	105 'i'	110 'n'	45 '-'	97 'a'	114 'r'	103 'g'	118 'v'	46 '.'
0x7fffffffe0c8:	101 'e'	120 'x'	101 'e'	0 '\000'	97 'a'	0 '\000'	120 'x'	121 'y'
0x7fffffffe0d0:	0 '\000'	100 'd'	0 '\000'	80 'P'	87 'W'	68 'D'	61 '='	47 '/'
0x7fffffffe0d8:	104 'h'	111 'o'	109 'm'	101 'e'	47 '/'	108 'l'	97 'a'	98 'b'
0x7fffffffe0e0:	105 'i'	108 'l'	101 'e'	47 '/'
```


```log
(gdb) info proc mappings 
process 6815
Mapped address spaces:

          Start Addr           End Addr       Size     Offset  Perms  objfile
      0x555555554000     0x555555555000     0x1000        0x0  r--p   /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
      0x555555555000     0x555555556000     0x1000     0x1000  r-xp   /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
      0x555555556000     0x555555557000     0x1000     0x2000  r--p   /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
      0x555555557000     0x555555558000     0x1000     0x2000  r--p   /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
      0x555555558000     0x555555559000     0x1000     0x3000  rw-p   /home/labile/devops-insider/docs/source/low-tec/gdb/main-argv/main-argv.exe
      0x555555559000     0x55555557a000    0x21000        0x0  rw-p   [heap]
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


# find [/sn] start_addr, end_addr, val1 [, val2, …]

find 0x7ffffffde000, 0x7ffffffff000, 0x7fffffffe080
warning: Unable to access 7162 bytes of target memory at 0x7fffffffd407, halting search.
Pattern not found

# But, and this is really important, the address 0x7ffffffff000 is NOT the actual end address. It is the address of the first byte that is not mapped! Using this address in our search will result in an error because it searches outside of the mapped address range. I show you that in my video. So, do step one byte back; 0x7fffffffefff


(gdb) find 0x7ffffffde000, 0x7fffffffefff, 0x7fffffffe080, 0x7fffffffe0cc#char* val on args[1]
0x7fffffffdcc8
1 pattern found.
(gdb) bt
#0  0x00007ffff7e610fb in execve () at ../sysdeps/unix/syscall-template.S:120
#1  0x00007ffff7e61615 in __GI_execl (path=<optimized out>, arg=0x55555555600a "ls") at ./posix/execl.c:56
#2  0x00005555555551e8 in main (argc=4, argv=0x7fffffffdcc8) at main-argv.c:1


```

### get env
```bash
pid=`pgrep main-argv.exe`&& e=(`cat /proc/$pid/stat`) && echo ${e[50]}
140737488347347

x/100bc 140737488347347

(gdb) x/100bc 140737488347347
0x7fffffffe0d3:	80 'P'	87 'W'	68 'D'	61 '='	47 '/'	104 'h'	111 'o'	109 'm'
0x7fffffffe0db:	101 'e'	47 '/'	108 'l'	97 'a'	98 'b'	105 'i'	108 'l'	101 'e'
0x7fffffffe0e3:	47 '/'	100 'd'	101 'e'	118 'v'	111 'o'	112 'p'	115 's'	45 '-'
0x7fffffffe0eb:	105 'i'	110 'n'	115 's'	105 'i'	100 'd'	101 'e'	114 'r'	47 '/'
0x7fffffffe0f3:	100 'd'	111 'o'	99 'c'	115 's'	47 '/'	115 's'	111 'o'	117 'u'
0x7fffffffe0fb:	114 'r'	99 'c'	101 'e'	47 '/'	108 'l'	111 'o'	119 'w'	45 '-'
0x7fffffffe103:	116 't'	101 'e'	99 'c'	47 '/'	103 'g'	100 'd'	98 'b'	47 '/'
0x7fffffffe10b:	109 'm'	97 'a'	105 'i'	110 'n'	45 '-'	97 'a'	114 'r'	103 'g'
0x7fffffffe113:	118 'v'	0 '\000'	76 'L'	65 'A'	78 'N'	71 'G'	85 'U'	65 'A'
0x7fffffffe11b:	71 'G'	69 'E'	61 '='	101 'e'	110 'n'	95 '_'	85 'U'	83 'S'
0x7fffffffe123:	0 '\000'	80 'P'	65 'A'	80 'P'	69 'E'	82 'R'	83 'S'	73 'I'
0x7fffffffe12b:	90 'Z'	69 'E'	61 '='	108 'l'	101 'e'	116 't'	116 't'	101 'e'
0x7fffffffe133:	114 'r'	0 '\000'	76 'L'	65 'A'

(gdb) find 0x7ffffffde000, 0x7fffffffefff, 0x7fffffffe0d3
0x7fffffffdcf0

(gdb) p *(((char**)0x7fffffffdcf0)+1)
$4 = 0x7fffffffe115 "LANGUAGE=en_US"
(gdb) p *(((char**)0x7fffffffdcf0)+2)
$5 = 0x7fffffffe124 "PAPERSIZE=letter"


call (int) execve((char *)0x7fffffffe080, (char**)0x7fffffffdcc8, (char**)0x7fffffffdcf0)
```


### syscall inject without libc
```
p $rip

set $rax=0x3b
set $rdi=0x7fffffffe080
set $rsi=0x7fffffffdcc8
set $rdx=0x7fffffffdcf0
#syscall
set {short}$rip = 0x050f
cont
```

### syscall inject without libc on container

```bash
ssh labile@192.168.122.55 #  ssh 到运行的 worker node

export POD="fortio-server-l2-0"
fortio_pids=$(pgrep fortio)
while IFS= read -r fortio_pid; do
    HN=$(sudo nsenter -u -t $fortio_pid hostname)
    if [[ "$HN" = "$POD" ]]; then # space between = is important
        sudo nsenter -u -t $fortio_pid hostname
        export POD_PID=$fortio_pid
    fi
done <<< "$fortio_pids"
echo $POD_PID
export PID=$POD_PID

sudo ldd /proc/$PID/root/usr/bin/fortio
	not a dynamic executable


pid=$PID && a=(`sudo cat /proc/$pid/stat`) && echo ${a[48]} 
140737488348939

pid=$PID && e=(`sudo cat /proc/$pid/stat`) && echo ${e[50]} 
140737488348999

```


```log
sudo gdb -p $PID

(gdb) shell ps -f -p 3589
UID          PID    PPID  C STIME TTY          TIME CMD
root        3589    3080  0 02:32 ?        00:00:00 /usr/bin/fortio server -M 8070 http://fortio-server-l2:8080


(gdb) info proc
process 3589

(gdb) shell ls -l /proc/3589/fd
total 0
lrwx------ 1 root root 64 Jun 22 01:24 0 -> /dev/null`
l-wx------ 1 root root 64 Jun 22 01:24 1 -> 'pipe:[36078]'
lrwx------ 1 root root 64 Jun 22 01:25 10 -> 'socket:[36963]'
lrwx------ 1 root root 64 Jun 22 01:25 11 -> 'socket:[36965]'
l-wx------ 1 root root 64 Jun 22 01:24 2 -> 'pipe:[36079]'
lrwx------ 1 root root 64 Jun 22 01:25 3 -> 'socket:[36951]'
lrwx------ 1 root root 64 Jun 22 01:25 4 -> 'anon_inode:[eventpoll]'
lr-x------ 1 root root 64 Jun 22 01:25 5 -> 'pipe:[36522]'
l-wx------ 1 root root 64 Jun 22 01:24 6 -> 'pipe:[36522]'
lrwx------ 1 root root 64 Jun 22 01:25 7 -> 'socket:[36959]'
lrwx------ 1 root root 64 Jun 22 01:25 8 -> 'socket:[36960]'
lrwx------ 1 root root 64 Jun 22 01:25 9 -> 'socket:[36962]'

set $rax=0x03
set $rdi=10
#syscall
set {short}$rip = 0x050f
stepi

(gdb) shell ls -l /proc/3589/fd
total 0
lrwx------ 1 root root 64 Jun 22 01:24 0 -> /dev/null
l-wx------ 1 root root 64 Jun 22 01:24 1 -> 'pipe:[36078]'
lrwx------ 1 root root 64 Jun 22 01:25 11 -> 'socket:[36965]'
..
lrwx------ 1 root root 64 Jun 22 01:25 9 -> 'socket:[36962]'


...
set $rax=0x03
set $rdi=3
#syscall
set {short}$rip = 0x050f
stepi
....

(gdb) shell ls -l /proc/3589/fd
total 0
lrwx------ 1 root root 64 Jun 22 01:24 0 -> /dev/null
l-wx------ 1 root root 64 Jun 22 01:24 1 -> 'pipe:[36078]'
l-wx------ 1 root root 64 Jun 22 01:24 2 -> 'pipe:[36079]'


(gdb) info proc mappings 
process 3589
Mapped address spaces:

          Start Addr           End Addr       Size     Offset objfile
            0x400000           0x9b1000   0x5b1000        0x0 /usr/bin/fortio
            0x9b1000           0xf8b000   0x5da000   0x5b1000 /usr/bin/fortio
            0xf8b000           0xfeb000    0x60000   0xb8b000 /usr/bin/fortio
            0xfeb000          0x102c000    0x41000        0x0 [heap]
        0xc000000000       0xc000400000   0x400000        0x0 
...
      0x7ffff7f9b000     0x7ffff7ffb000    0x60000        0x0 
      0x7ffff7ffb000     0x7ffff7ffe000     0x3000        0x0 [vvar]
      0x7ffff7ffe000     0x7ffff7fff000     0x1000        0x0 [vdso]
      0x7ffffffde000     0x7ffffffff000    0x21000        0x0 [stack]

(gdb) p (void*)140737488348939
$4 = (void *) 0x7fffffffe70b

(gdb) p (void*)140737488348999
$4 = (void *) 0x7fffffffe747

(gdb) x/100bc 0x7fffffffe70b
0x7fffffffe70b:	47 '/'	117 'u'	115 's'	114 'r'	47 '/'	98 'b'	105 'i'	110 'n'
0x7fffffffe713:	47 '/'	102 'f'	111 'o'	114 'r'	116 't'	105 'i'	111 'o'	0 '\000'


find 0x7ffffffde000, 0x7fffffffefff, 0x7fffffffe70b
0x7fffffffe3b8

find 0x7ffffffde000, 0x7fffffffefff, 0x7fffffffe747
0x7fffffffe3e0

set $rax=0x3b
set $rdi=0x7fffffffe70b
set $rsi=0x7fffffffe3b8
set $rdx=0x7fffffffe3e0
#syscall
set {short}$rip = 0x050f
stepi
```



```log
labile@labile-T30 ➜ labile $ k logs fortio-server-l2-0
01:24:54 I scli.go:90> Starting Φορτίο 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux
01:24:54 Fortio 1.54.3 tcp-echo server listening on tcp [::]:8078
01:24:54 Fortio 1.54.3 udp-echo server listening on udp [::]:8078
01:24:54 Fortio 1.54.3 grpc 'ping' server listening on tcp [::]:8079
01:24:54 Fortio 1.54.3 https redirector server listening on tcp [::]:8081
01:24:54 Fortio 1.54.3 http-echo server listening on tcp [::]:8080
01:24:54 Data directory is /var/lib/fortio
01:24:54 REST API on /fortio/rest/run, /fortio/rest/status, /fortio/rest/stop, /fortio/rest/dns
01:24:54 Debug endpoint on /debug, Additional Echo on /debug/echo/, Flags on /fortio/flags, and Metrics on /debug/metrics
01:24:54 Fortio 1.54.3 Multi on 8070 server listening on tcp [::]:8070
01:24:54 I http_forwarder.go:288> Multi-server on [::]:8070 running with &{Targets:[{Destination:http://fortio-server-l2:8080 MirrorOrigin:true}] Serial:false Name:Multi on [::]:8070 client:0xc0001f0f00}
01:24:54 I fortio_main.go:292> All fortio 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux servers started!
	 UI started - visit:
		http://localhost:8080/fortio/
	 (or any host/ip reachable on this server)
labile@labile-T30 ➜ labile $ k logs -f  fortio-server-l2-0
01:24:54 I scli.go:90> Starting Φορτίο 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux
01:24:54 Fortio 1.54.3 tcp-echo server listening on tcp [::]:8078
01:24:54 Fortio 1.54.3 udp-echo server listening on udp [::]:8078
01:24:54 Fortio 1.54.3 grpc 'ping' server listening on tcp [::]:8079
01:24:54 Fortio 1.54.3 https redirector server listening on tcp [::]:8081
01:24:54 Fortio 1.54.3 http-echo server listening on tcp [::]:8080
01:24:54 Data directory is /var/lib/fortio
01:24:54 REST API on /fortio/rest/run, /fortio/rest/status, /fortio/rest/stop, /fortio/rest/dns
01:24:54 Debug endpoint on /debug, Additional Echo on /debug/echo/, Flags on /fortio/flags, and Metrics on /debug/metrics
01:24:54 Fortio 1.54.3 Multi on 8070 server listening on tcp [::]:8070
01:24:54 I http_forwarder.go:288> Multi-server on [::]:8070 running with &{Targets:[{Destination:http://fortio-server-l2:8080 MirrorOrigin:true}] Serial:false Name:Multi on [::]:8070 client:0xc0001f0f00}
01:24:54 I fortio_main.go:292> All fortio 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux servers started!
	 UI started - visit:
		http://localhost:8080/fortio/
	 (or any host/ip reachable on this server)


03:08:08 I scli.go:90> Starting Φορτίο 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux
03:08:08 Fortio 1.54.3 tcp-echo server listening on tcp [::]:8078
03:08:08 Fortio 1.54.3 udp-echo server listening on udp [::]:8078
03:08:08 Fortio 1.54.3 grpc 'ping' server listening on tcp [::]:8079
03:08:08 Fortio 1.54.3 https redirector server listening on tcp [::]:8081
03:08:08 Fortio 1.54.3 http-echo server listening on tcp [::]:8080
03:08:08 Data directory is /var/lib/fortio
03:08:08 REST API on /fortio/rest/run, /fortio/rest/status, /fortio/rest/stop, /fortio/rest/dns
03:08:08 Debug endpoint on /debug, Additional Echo on /debug/echo/, Flags on /fortio/flags, and Metrics on /debug/metrics
03:08:08 Fortio 1.54.3 Multi on 8070 server listening on tcp [::]:8070
03:08:08 I http_forwarder.go:288> Multi-server on [::]:8070 running with &{Targets:[{Destination:http://fortio-server-l2:8080 MirrorOrigin:true}] Serial:false Name:Multi on [::]:8070 client:0xc000254f00}
03:08:08 I fortio_main.go:292> All fortio 1.54.3 h1:c9WIOtp4A2lSvDLs1Y01S6yNirtAvaBJJnTzcv/9G/M= go1.20.4 amd64 linux servers started!
	 UI started - visit:
		http://localhost:8080/fortio/
	 (or any host/ip reachable on this server)

```