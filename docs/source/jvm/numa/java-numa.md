

> https://ionutbalosin.com/2020/01/hotspot-jvm-performance-tuning-guidelines/

* -XX:+UseNUMA (default false)

Improve the performance of a JVM that runs on a machine with nonuniform memory architecture (NUMA), or multiple sockets, by enabling the NUMA aware Collector to allocate objects in memory node local to a processor, increasing the application’s use of lower latency memory.

Note: at the moment the NUMA aware Collectors are: **Parallel GC, G1 GC, and ZGC**.




### G1 GC NUMA-Aware

> https://sangheon.github.io/2020/11/03/g1-numa.html
> 
> The default Garbage Collector, G1 GC was enhanced on JDK-14 by making its memory allocator NUMA-aware by [JEP-345: NUMA-Aware Memory Allocation for G1](https://bugs.openjdk.java.net/browse/JDK-8210473) ^[1](https://sangheon.github.io/2020/11/03/g1-numa.html#fn:numa_impl)^.
In this article, I will explain a little bit about its implementation. I will give minimal explanations about how G1 GC works in general.


#### For Java 8 G1 GC
> https://www.mail-archive.com/hotspot-gc-use@openjdk.java.net/msg00390.html

> G1 is not NUMA aware. Only Parallel GC(-XX:+UseParallelGC) has NUMA awareness. 

> May I use the -XX:+UseNUMA flag with -XX:+UseG1GC?

> Yes, you can use `+UseNUMA` with `+UseG1GC`.
> G1 algorithm is not NUMA aware, **but with UseNUMA enabled, we can get NUMA interleaving.** i.e. Interleave memory across NUMA nodes if possible and this would give some benefits on NUMA system. FYI, `UseNUMAInterleaving`(false by default) command-line option will be automatically enabled if `UseNUMA` is enabled. 

> https://docs.oracle.com/javase/7/docs/technotes/guides/vm/performance-enhancements-7.html
The NUMA-aware allocator can be turned on with the -XX:+UseNUMA flag in conjunction with the selection of the Parallel Scavenger garbage collector. The Parallel Scavenger garbage collector is the default for a server-class machine. The Parallel Scavenger garbage collector can also be turned on explicitly by specifying the `-XX:+UseParallelGC` option.

### parallel collector NUMA-Aware

> https://bugs.openjdk.java.net/browse/JDK-8210473

The parallel collector, enabled by by -XX:+UseParallelGC, has been NUMA-aware for many years. This has helped to improve the performance of configurations that run a single JVM across multiple sockets.

## Ref
https://frankdenneman.nl/2015/02/20/memory-deep-dive/