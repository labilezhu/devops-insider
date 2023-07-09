

> https://alidg.me/blog/2019/6/21/tlab-jvm

As it turns out, for even more faster allocations, JVM divides the Eden space into a few more sub-regions, each dedicated to a particular thread. Each of those dedicated regions is called Thread-Local Allocation Buffers or TLAB for short.

>    uintx MinTLABSize                               = 2048                                {product}

### TLAB Sizing

* * * * *

If the `-XX:TLABSize` flag was set to a positive number (Defaults to *zero*), then the [initial size of a TLAB](http://hg.openjdk.java.net/jdk8/jdk8/hotspot/file/87ee5ee27509/src/share/vm/memory/threadLocalAllocBuffer.cpp#l235) would be equal to `-XX:TLABSize` divided by *Heap Word Size*. Otherwise, the initial size would be a function of the average number of [allocating threads](http://hg.openjdk.java.net/jdk8/jdk8/hotspot/file/87ee5ee27509/src/share/vm/memory/threadLocalAllocBuffer.cpp#l240). Also, the TLAB size can't be:

-   More than a [maximum value](http://hg.openjdk.java.net/jdk8/jdk8/hotspot/file/87ee5ee27509/src/share/vm/memory/threadLocalAllocBuffer.cpp#l251).
-   Less than a minimum value determined by `-XX:MinTLABSize` flag.

If the `-XX:ResizeTLAB` is enabled (Which is by default), then JVM can adaptively [resize](http://hg.openjdk.java.net/jdk8/jdk8/hotspot/file/87ee5ee27509/src/share/vm/memory/threadLocalAllocBuffer.cpp#l134) the TLAB size. The new size is always depends on three major factors:

-   Number of application threads
-   Allocation rate
-   Eden Size


## Ref
https://alidg.me/blog/2019/6/21/tlab-jvm
https://blogs.oracle.com/jonthecollector/the-real-thing
https://chowdera.com/2021/04/20210412130159067f.html

* [Why Java's TLABs are so important and why write contention is a performance killer in multicore environments](https://www.opsian.com/blog/jvm-tlabs-important-multicore/)