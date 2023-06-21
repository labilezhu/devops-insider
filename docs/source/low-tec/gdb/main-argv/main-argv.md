


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