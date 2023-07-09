
> https://access.redhat.com/documentation/en-us/openjdk/8/pdf/using_shenandoah_garbage_collector_with_openjdk_8/openjdk-8-using_shenandoah_garbage_collector_with_openjdk_8-en-us.pdf

Shenandoah is the low pause time garbage collector (GC) that reduces GC pause times by performing
more garbage collection work concurrently with the running Java program. Concurrent Mark Sweep
garbage collector (CMS) and G1, default garbage collector for OpenJDK 8 perform concurrent marking
of live objects.

**Shenandoah adds concurrent compaction**. It reduces GC pause times by compacting objects
concurrently with running Java threads. **Pause times with Shenandoah are independent of the heap size**,
meaning you will have consistent pause time whether your heap is 200 MB or 200 GB. Shenandoah is an
algorithm for applications which require responsiveness and predictable short pauses. For more
information, see [Shenandoah: A Low-Pause-Time Garbage Collector](https://openjdk.java.net/jeps/189).



