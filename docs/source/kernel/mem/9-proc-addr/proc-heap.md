---
title: "Kernel - Process Heap"
date: 2021-02-26T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- kernel
- kernel-mem
- kernel-process
---


> [Understanding The Linux Kernel]

## Managing the Heap

Each Unix process owns a specific memory region called the heap, which is used to
satisfy the process’s dynamic memory requests. The start_brk and brk fields of the
memory descriptor delimit the starting and ending addresses, respectively, of that
region.
The following APIs can be used by the process to request and release dynamic memory:

- malloc(size)
Requests size bytes of dynamic memory; if the allocation succeeds, it returns the
linear address of the first memory location.
- calloc(n,size)
Requests an array consisting of n elements of size size ; if the allocation suc-
ceeds, it initializes the array components to 0 and returns the linear address of
the first element.
- realloc(ptr,size)
Changes the size of a memory area previously allocated by malloc() or calloc() .
- free(addr)
Releases the memory region allocated by malloc( ) or calloc( ) that has an ini-
tial address of addr .
- brk(addr)
Modifies the size of the heap directly; the addr parameter specifies the new value
of current->mm->brk , and the return value is the new ending address of the mem-
ory region (the process must check whether it coincides with the requested addr
value).
- sbrk(incr)
Is similar to brk() , except that the incr parameter specifies the increment or dec-
rement of the heap size in bytes.

The `brk()` function differs from the other functions listed because it is the only one
implemented as a system call. All the other functions are implemented in the C
library by using brk( ) and mmap( ) . *