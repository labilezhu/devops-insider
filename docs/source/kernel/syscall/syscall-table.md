# Syscall Table




> source: [Searchable Linux Syscall Table for x86 and x86_64](https://filippo.io/linux-syscall-table/)



**Instruction:** `syscall`

**Return value found in:** `%rax`

Syscalls are implemented in functions named as in the *Entry point* column, or with the `DEFINE_SYSCALLx(%name%` macro.

Relevant man pages: [`syscall(2)`](https://manpages.debian.org/unstable/manpages-dev/syscall.2.en.html), [`syscalls(2)`](https://manpages.debian.org/unstable/manpages-dev/syscalls.2.en.html)







x86 64bit:

|  NR  | syscall name |                          references                          | %rax | arg0 (%rdi)           | arg1 (%rsi)             | arg2 (%rdx)             | arg3 (%r10) | arg4 (%r8) | arg5 (%r9) |
| :--: | ------------ | :----------------------------------------------------------: | :--: | --------------------- | ----------------------- | ----------------------- | ----------- | ---------- | ---------- |
|  0   | read         | [man/](https://man7.org/linux/man-pages/man2/read.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*read) | 0x00 | unsigned int fd       | char \*buf              | size\_t count           | \-          | \-         | \-         |
|  1   | write        | [man/](https://man7.org/linux/man-pages/man2/write.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*write) | 0x01 | unsigned int fd       | const char \*buf        | size\_t count           | \-          | \-         | \-         |
|  2   | open         | [man/](https://man7.org/linux/man-pages/man2/open.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*open) | 0x02 | const char \*filename | int flags               | umode\_t mode           | \-          | \-         | \-         |
|  3   | close        | [man/](https://man7.org/linux/man-pages/man2/close.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*close) | 0x03 | unsigned int fd       | \-                      | \-                      | \-          | \-         | \-         |
|      | ...          |                                                              |      |                       |                         |                         |             |            |            |
|  59  | execve       | [man/](https://man7.org/linux/man-pages/man2/execve.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*execve) | 0x3b | const char *filename  | const char *const *argv | const char *const *envp | -           | -          | -          |
|  60  | exit         | [man/](https://man7.org/linux/man-pages/man2/exit.2.html) [cs/](https://source.chromium.org/search?ss=chromiumos&q=SYSCALL_DEFINE.*exit) | 0x3c | int error_code        | -                       | -                       | -           | -          | -          |
|      | ...          |                                                              |      |                       |                         |                         |             |            |            |

https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md



https://github.com/torvalds/linux/blob/v3.13/arch/x86/syscalls/syscall_64.tbl

```
#
# 64-bit system call numbers and entry vectors
#
# The format is:
# <number> <abi> <name> <entry point>
#
# The abi is "common", "64" or "x32" for this file.
#
0	common	read			sys_read
1	common	write			sys_write
2	common	open			sys_open
3	common	close			sys_close
```





x86 32bit:

https://github.com/torvalds/linux/blob/v3.13/arch/x86/syscalls/syscall_32.tbl

```
#
# 32-bit system call numbers and entry vectors
#
# The format is:
# <number> <abi> <name> <entry point> <compat entry point>
#
# The abi is always "i386" for this file.
#
0	i386	restart_syscall		sys_restart_syscall
1	i386	exit			sys_exit
2	i386	fork			sys_fork			stub32_fork
3	i386	read			sys_read
4	i386	write			sys_write
5	i386	open			sys_open			compat_sys_open
6	i386	close			sys_close
```





