

## Concept

> https://medium.com/nerds-malt/java-in-k8s-how-weve-reduced-memory-usage-without-changing-any-code-cbef5d740ad

***Arena***: *A structure that is***shared among one or more threads*** which contains references to one or more heaps, as well as ***linked lists of chunks within those heaps which are "free"***. Threads assigned to each arena will allocate memory from that arena's free lists.*

***Heap***: *A contiguous region of ***memory*** that is ***subdivided into chunks*** to be allocated. Each heap belongs to exactly one arena.*

***Chunk***: *A* **small range of memory that can be allocated** *(owned by the application), ***freed*** (owned by glibc), or ***combined with adjacent chunks*** into larger ranges. Note that a chunk is a wrapper around the block of memory that is given to the application. Each chunk exists in one heap and belongs to one arena.*

What is important here is that malloc will always **free memory from the top of the heap** it means that if a chunk has to be freed but it isn't at the top of the heap, it won't be released at the OS level.

> https://sourceware.org/glibc/wiki/MallocInternals

## concurrent

To remain efficient in a highly concurrent system, multiple arenas are used:

-   If a thread needs to allocate a new chunk of memory, malloc will lock an arena to allocate memory (you can see it [here](https://github.com/lattera/glibc/blob/master/malloc/arena.c#L118) in the malloc source code).
-   If an arena is locked, it will try with another arena, and so on...
-   If all arenas are locked, it must wait for an arena to become usable.

> Another version: https://blog.cloudflare.com/the-effect-of-switching-to-tcmalloc-on-rocksdb-memory-use/

-   A thread tries to get a chunk of memory from an arena it used last time, in order to do that it acquires an exclusive lock for the arena
-   If the lock is held by another thread, it tries the next arena
-   If all arenas were locked it creates a new arena and uses memory from it
-   There is a limit on the number of arenas - eight arenas per core


## arena

I found that there are many strange 64MB memory mappings. Checking the data found that this was caused by the new arena feature introduced by glibc in version 2.10 . CentOS 6/7 of glibc mostly 2.12/2.17 , so will have this problem. This function allocates a local arena for each thread to speed up the execution of multiple threads.

In glibc of arena.c used in the mmap () call on the previous sample code and the like:

    p2 = (char *)mmap(aligned_heap_area, HEAP_MAX_SIZE, PROT_NONE,

                          MAP_NORESERVE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)

After that, only a small part of the address is mapped to physical memory:

    mprotect(p2, size, PROT_READ | PROT_WRITE)

So in a multi-threaded program, there will be quite a lot of arena of 64MB allocated. This can be controlled with the environment variable `MALLOC_ARENA_MAX` . The default value in a 64 -bit system is 128 .

> https://blog.titanwolf.in/a?ID=00850-86fd376c-3cae-400e-887e-539a089dbc2f
> https://www.cnblogs.com/seasonsluo/p/java_virt.html
> https://stevecao.wordpress.com/2018/11/02/anatomy-of-jvm-memory/



#### GLIBC malloc
> https://publib.boulder.ibm.com/httpserv/cookbook/Operating_Systems-Linux.html?lang=en

In recent kernels, the text is at the bottom, stack at the top, and mmap/heap sections grow towards each other in a shared space (although they cannot overlap). By default, the malloc implementation in glibc (which was based on ptmalloc, which in turn was based on dlmalloc) will allocate into either the native heap (sbrk) or mmap space, based on various heuristics and thresholds: If there's enough free space in the native heap, allocate there. Otherwise, if the allocation size is greater than some threshold (slides between 128KB and 32/64MB based on various factors [1]), allocate a private, anonymous mmap instead of native heap (mmap isn't limited by ulimit -d) (<https://www.kernel.org/doc/man-pages/online/pages/man3/mallopt.3.html>)

In the raw call of sbrk versus mmap, mmap is slower because it must zero the range of bytes (http://www.sourceware.org/ml/libc-alpha/2006-03/msg00033.html).

## Options

官方说明：
> https://www.gnu.org/software/libc/manual/html_node/Memory-Allocation-Tunables.html
> https://man7.org/linux/man-pages/man3/mallopt.3.html

For security
       reasons, these variables are ignored in set-user-ID and set-
       group-ID programs.

       MALLOC_ARENA_MAX
              If this parameter has a nonzero value, it defines a hard
              limit on the maximum number of arenas that can be created.
              An arena represents a pool of memory that can be used by
              malloc(3) (and similar) calls to service allocation
              requests.  Arenas are thread safe and therefore may have
              multiple concurrent memory requests.  The trade-off is
              between the number of threads and the number of arenas.
              The more arenas you have, the lower the per-thread
              contention, but the higher the memory usage.

              The default value of this parameter is 0, meaning that the
              limit on the number of arenas is determined according to
              the setting of M_ARENA_TEST.

              This parameter has been available since glibc 2.10 via
              --enable-experimental-malloc, and since glibc 2.15 by
              default.  In some versions of the allocator there was no
              limit on the number of created arenas (e.g., CentOS 5,
              RHEL 5).

              When employing newer glibc versions, applications may in
              some cases exhibit high contention when accessing arenas.
              In these cases, it may be beneficial to increase
              M_ARENA_MAX to match the number of threads.  This is
              similar in behavior to strategies taken by tcmalloc and
              jemalloc (e.g., per-thread allocation pools).

       MALLOC_ARENA_TEST
              This parameter specifies a value, in number of arenas
              created, at which point the system configuration will be
              examined to determine a hard limit on the number of
              created arenas.  (See M_ARENA_MAX for the definition of an
              arena.)

              The computation of the arena hard limit is implementation-
              defined and is usually calculated as a multiple of the
              number of available CPUs.  Once the hard limit is
              computed, the result is final and constrains the total
              number of arenas.

              The default value for the M_ARENA_TEST parameter is 2 on
              systems where sizeof(long) is 4; otherwise the default
              value is 8.

              This parameter has been available since glibc 2.10 via
              --enable-experimental-malloc, and since glibc 2.15 by
              default.

              The value of M_ARENA_TEST is not used when M_ARENA_MAX has
              a nonzero value.

       MALLOC_MMAP_THRESHOLD_
              For allocations greater than or equal to the limit
              specified (in bytes) by M_MMAP_THRESHOLD that can't be
              satisfied from the free list, the memory-allocation
              functions employ mmap(2) instead of increasing the program
              break using sbrk(2).

              Allocating memory using mmap(2) has the significant
              advantage that the allocated memory blocks can always be
              independently released back to the system.  (By contrast,
              the heap can be trimmed only if memory is freed at the top
              end.)  On the other hand, there are some disadvantages to
              the use of mmap(2): deallocated space is not placed on the
              free list for reuse by later allocations; memory may be
              wasted because mmap(2) allocations must be page-aligned;
              and the kernel must perform the expensive task of zeroing
              out memory allocated via mmap(2).  Balancing these factors
              leads to a default setting of 128*1024 for the
              M_MMAP_THRESHOLD parameter.

              The lower limit for this parameter is 0.  The upper limit
              is DEFAULT_MMAP_THRESHOLD_MAX: 512*1024 on 32-bit systems
              or 4*1024*1024*sizeof(long) on 64-bit systems.

              Note: Nowadays, glibc uses a dynamic mmap threshold by
              default.  The initial value of the threshold is 128*1024,
              but when blocks larger than the current threshold and less
              than or equal to DEFAULT_MMAP_THRESHOLD_MAX are freed, the
              threshold is adjusted upward to the size of the freed
              block.  When dynamic mmap thresholding is in effect, the
              threshold for trimming the heap is also dynamically
              adjusted to be twice the dynamic mmap threshold.  Dynamic
              adjustment of the mmap threshold is disabled if any of the
              M_TRIM_THRESHOLD, M_TOP_PAD, M_MMAP_THRESHOLD, or
              M_MMAP_MAX parameters is set.


##### MALLOC_ARENA_MAX

Starting with glibc 2.11 (for example, customers upgrading from RHEL 5 to RHEL 6), by default, when glibc malloc detects mutex contention (i.e. concurrent mallocs), then the native malloc heap is broken up into sub-pools called arenas. This is achieved by assigning threads their own memory pools and by avoiding locking in some situations. The amount of additional memory used for the memory pools (if any) can be controlled using the environment variables MALLOC_ARENA_TEST and MALLOC_ARENA_MAX. `MALLOC_ARENA_TEST` specifies that a test for the number of cores is performed once the number of memory pools reaches this value. `MALLOC_ARENA_MAX` sets the maximum number of memory pools used, regardless of the number of cores.

* The default maximum arena size is 1MB on 32-bit and 64MB on 64-bit.
```bash
# 64MB arena
pmap 27257 | grep 65404 | wc -l
235

pmap -x 27257 | sort -k 3 -n -r

# 这里发现一个规律，65484 + 52 = 65536 KB, 65420 + 116 = 65536 KB, 65036 + 500 = 65536 KB
```

* The **default maximum number of arenas is the number of cores multiplied by 2 for 32-bit and 8 for 64-bit**.



This can increase fragmentation because the free trees are separate.

In principle, the net performance impact should be positive of per thread arenas, but testing different arena numbers and sizes may result in performance improvements depending on your workload.

You can revert the arena behavior with the environment variable MALLOC_ARENA_MAX=1

#### glibc and Java

> https://medium.com/nerds-malt/java-in-k8s-how-weve-reduced-memory-usage-without-changing-any-code-cbef5d740ad

#### memory fragmentation and RSS
> https://blog.cloudflare.com/the-effect-of-switching-to-tcmalloc-on-rocksdb-memory-use/



#### glibc & container 
https://systemadminspro.com/java-in-docker-cpu-limits-server-class-machine/
https://groups.google.com/g/orthanc-users/c/qWqxpvCPv8g/m/47wnYyhOCAAJ?pli=1


## Troubleshooting

> http://tukan.farm/2016/07/27/munmap-madness/

> [malloc_stats()](https://man7.org/linux/man-pages/man3/malloc_stats.3.html)
> [mallinfo(3)](https://man7.org/linux/man-pages/man3/mallinfo.3.html)
> [malloc_trim](https://man7.org/linux/man-pages/man3/malloc_trim.3.html)

#### malloc_trim
The **malloc_trim**() function attempts to release free memory from
the heap (by calling [sbrk(2)](https://man7.org/linux/man-pages/man2/sbrk.2.html) or [madvise(2)](https://man7.org/linux/man-pages/man2/madvise.2.html) with suitable
arguments).

#### gdb

> https://stackoverflow.com/questions/2564752/examining-c-c-heap-memory-statistics-in-gdb
> https://codearcana.com/posts/2016/07/11/arena-leak-in-glibc.html

```
(gdb) call malloc_stats()
Arena 0:
system bytes     =     135168
in use bytes     =         96
Total (incl. mmap):
system bytes     =     135168
in use bytes     =         96
max mmap regions =          0
max mmap bytes   =          0


(gdb) call malloc_info(0, stdout)
<malloc version="1">
<heap nr="0">
<sizes>
<unsorted from="1228788" to="1229476" total="3917678" count="3221220448"/>
</sizes>
<total type="fast" count="0" size="0"/>
<total type="rest" count="3221220448" size="3917678"/>
<system type="current" size="135168"/>
<system type="max" size="135168"/>
<aspace type="total" size="135168"/>
<aspace type="mprotect" size="135168"/>
</heap>
<total type="fast" count="0" size="0"/>
<total type="rest" count="3221220448" size="3917678"/>
<system type="current" size="135168
/>
<system type="max" size="135168
/>
<aspace type="total" size="135168"/>
<aspace type="mprotect" size="135168"/>
</malloc>

nsenter -t $PID -p -i -n

sudo gdb --batch --pid $PID --ex 'call malloc_stats()'

sudo gdb --batch --pid $PID --ex 'call malloc_info(0, stdout)'



```


> https://blog.csdn.net/zzhongcy/article/details/89135056




## Ref
https://azeria-labs.com/heap-exploitation-part-1-understanding-the-glibc-heap-implementation/
https://developers.redhat.com/blog/2017/03/02/malloc-internals-and-you#introduction
https://sourceware.org/glibc/wiki/MallocInternals
https://www.gnu.org/software/libc/manual/html_node/Tunables.html
https://github.com/cloudfoundry/java-buildpack/issues/320
https://cloud.tencent.com/developer/article/1622195
https://www.tutorialdocs.com/article/spring-boot-memory-leak.html
https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo

