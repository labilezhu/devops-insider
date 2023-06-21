# gdb inject syscall



## Dissecting killbutmakeitlooklikeanaccident.sh

> source: [https://thomasw.dev/post/killbutmakeitlooklikeanaccident/](https://thomasw.dev/post/killbutmakeitlooklikeanaccident/)



omeone pointed out a shell script that was posted to [GitHub](https://github.com/timb-machine-mirrors/killbutmakeitlooklikeanaccident.sh/blob/0bf35bd22fd2fa56a4205baebd6ddb30bc9848b5/killbutmakeitlooklikeanaccident.sh) named ‘killbutmakeitlooklikeanaccident.sh’.

It contains a single command:

```sh
#!/bin/bash

gdb -p "$1" -batch -ex 'set {short}$rip = 0x050f' -ex 'set $rax=231' -ex 'set $rdi=0' -ex 'cont'
```

If you run it, passing a PID as the first argument, the target process terminates with exit code 0. Neat!

This is distinctively different from using the `kill` command, which sends a SIGTERM to the target processes, causing signal handlers to be called and generally makes the process terminate with exit code 143.

This script only works on x86-64.

### How does it work?

The shell script executes `gdb`, the GNU Debugger, in batch mode (`-batch`) and tells it to attach to the target process (`-p`). Then, through the `-ex` arguments, executes the following script:

```
set {short}$rip = 0x050f
set $rax=231
set $rdi=0
cont
```

Let’s look at it line-by-line:

#### `set {short}$rip = 0x050f`

This line writes `0x050f` to the *location RIP is pointing to*. GDB syntax can be confusing! This construct (`{type}`) is covered in the manual under [Expressions](https://web.mit.edu/gnu/doc/html/gdb_toc.html#SEC54). Basically, it writes a value to an object of a type (in this case, `short`) at the provided *address*. So; it writes `0f 05` (corrected for endianness) at the location RIP is pointing to, which is the next instruction the CPU will execute when the program resumes. Using `rasm2` from radare2, we can quickly determine the instruction being executed:

```
  $ rasm2 -a x86 -d 0f05
  syscall
```

The target process will execute a `syscall` instruction next.

#### `set $rax=231`

[syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html) states that, for x86-64, the system call number should be loaded into the RAX register.

We can check which system call has number 231:

```
$ grep 231 /usr/include/x86_64-linux-gnu/asm/unistd_64.h
#define __NR_exit_group 231
```

The [exit_group](https://man7.org/linux/man-pages/man2/exit_group.2.html) system call which terminates all threads in the process.

#### `set $rdi=0`

According to [syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html) the first argument for the system call is stored in the RDI register. According to [exit_group(2)](https://man7.org/linux/man-pages/man2/exit_group.2.html) the system call has one argument: the process exit code.

#### `cont`

The GDB `cont` command is shorthand for `continue` (you can also use the even shorter `c`). This resumes the process, from which point the process will execute the exit_group system call and terminate with exit code 0.


## Killing with ptrace

> source [https://www.mjr19.org.uk/IT/ptrace.html](https://www.mjr19.org.uk/IT/ptrace.html)



I found myself wanting to kill a process under Linux, but have it exit with a zero return code, so that its parent thought that it had no error. Impossible using `kill` directly, but possible with `ptrace`. The use case was stopping a component of the Debian/Ubuntu install system such that it would not be rerun as a failure.

The idea is to use `ptrace` to attach to the process, to stop the process, alter the next instruction pointed to by the instruction pointer to `syscall`, and to set the registers to indicate a function of 60 (exit), with a return code, held in %rdi, of zero. The traced process is then resumed.

The code will run as root, or will run against one's own processes if `/proc/sys/kernel/yama/ptrace_scope` is zero, which it is not on most modern Linux distributions. It assumes x86_64, and therefore that the op code for syscall is 0x0f05, and that storage is little-endian, so that a word of any length set to 0x050f will be written to memory as 0x0f, followed by 0x05, probably followed by several bytes of zeroes.

The code can be considered a very basic demonstration of what `ptrace` is capable of.

```
#include<stdio.h>
#include<stdlib.h>
#include<sys/ptrace.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/user.h>
#include<errno.h>

int main(int argc, char * argv[]){
  pid_t pid;
  long err;
  int status;
  struct user_regs_struct regs;
  
  pid=atoi(argv[1]);
  err=ptrace(PTRACE_ATTACH,pid,NULL,NULL);
  if (err) {perror(NULL); exit(1);}
  fprintf(stderr,"Successfully attached\n");

  waitpid(pid,&status,WUNTRACED);
  fprintf(stderr,"Wait over\n");

  ptrace(PTRACE_GETREGS, pid, NULL, ®s);
  if (err) {perror(NULL); exit(1);}
  fprintf(stderr,"Registers fetched\n");

  regs.rax=60;
  regs.rdi=0;
  ptrace(PTRACE_SETREGS, pid, NULL, ®s);
  if (err) {perror(NULL); exit(1);}

  fprintf(stderr,"Registers set\n");
  ptrace(PTRACE_POKETEXT, pid, (void*)regs.rip, (void*)0x050f);
  
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  fprintf(stderr,"Target resumes\n");
  exit(0);
}
```

To demonstrate its use, try typing
`sleep 100; echo $?`
in one window, and in another find the PID of the sleep command, and kill it. If killed with simply `kill`, the response in the first window will be

```
Terminated
143
```

whereas if killed with this trick, the response will be simply

```
0
```

showing that the sleep exited with no error. (Note that the error code on exiting due to a signal is 128 + signal number. Here the signal number is 15, being SIGTERM, the default signal sent by `kill`, so the return code is 143.)



