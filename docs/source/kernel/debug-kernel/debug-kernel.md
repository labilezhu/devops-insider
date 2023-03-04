> Ref. [Learning Linux Binary Analys]

## Useful devices and files

### /proc/<pid>/maps
/proc/<pid>/maps file contains the layout of a process image by showing each
memory mapping. This includes the executable, shared libraries, stack, heap, VDSO,
and more. This file is critical for being able to quickly parse the layout of a process
address space and is used more than once throughout this book.


### /proc/kcore
The /proc/kcore is an entry in the proc filesystem that acts as a dynamic core file
of the Linux kernel. That is, it is a raw dump of memory that is presented in the form
of an ELF core file that can be used by GDB to debug and analyze the kernel. We will
explore /proc/kcore in depth in Chapter 9, Linux /proc/kcore Analysis.

### /boot/System.map
This file is available on almost all Linux distributions and is very useful for kernel
hackers. It contains every symbol for the entire kernel.

### /proc/kallsyms
The kallsyms is very similar to System.map , except that it is a /proc entry that
means that it is maintained by the kernel and is dynamically updated. Therefore, if
any new LKMs are installed, the symbols will be added to /proc/kallsyms on the
fly. The /proc/kallsyms contains at least most of the symbols in the kernel and will
contain all of them if specified in the CONFIG_KALLSYMS_ALL kernel config.

### /proc/iomem
The iomem is a useful proc entry as it is very similar to /proc/<pid>/maps , but for
all of the system memory. If, for instance, you want to know where the kernel's text
segment is mapped in the physical memory, you can search for the Kernel string
and you will see the code/text segment, the data segment, and the bss segment:
$ grep Kernel /proc/iomem
01000000-016d9b27 : Kernel code
016d9b28-01ceeebf : Kernel data
01df0000-01f26fff : Kernel bss

