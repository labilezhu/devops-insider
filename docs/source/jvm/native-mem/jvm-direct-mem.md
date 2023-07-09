

> https://developer.aliyun.com/article/2948

**广义的堆外内存**

说到堆外内存，那大家肯定想到堆内内存，这也是我们大家接触最多的，我们在jvm参数里通常设置-Xmx来指定我们的堆的最大值，不过这还不是我们理解的Java堆，-Xmx的值是新生代和老生代的和的最大值，我们在jvm参数里通常还会加一个参数-XX:MaxPermSize来指定持久代的最大值，那么我们认识的Java堆的最大值其实是-Xmx和-XX:MaxPermSize的总和，在分代算法下，新生代，老生代和持久代是连续的虚拟地址，因为它们是一起分配的，那么剩下的都可以认为是堆外内存(广义的)了，这些包括了jvm本身在运行过程中分配的内存，codecache，jni里分配的内存，DirectByteBuffer分配的内存等等

**狭义的堆外内存**

而作为java开发者，我们常说的堆外内存溢出了，其实是狭义的堆外内存，这个主要是指java.nio.DirectByteBuffer在创建的时候分配内存，我们这篇文章里也主要是讲狭义的堆外内存，因为它和我们平时碰到的问题比较密切

JDK/JVM里DirectByteBuffer的实现
```java
    DirectByteBuffer(int cap) {                   // package-private

        super(-1, 0, cap, cap);
        boolean pa = VM.isDirectMemoryPageAligned();
        int ps = Bits.pageSize();
        long size = Math.max(1L, (long)cap + (pa ? ps : 0));
        Bits.reserveMemory(size, cap);

        long base = 0;
        try {
            base = unsafe.allocateMemory(size);
        } catch (OutOfMemoryError x) {
            Bits.unreserveMemory(size, cap);
            throw x;
        }
        unsafe.setMemory(base, size, (byte) 0);
        if (pa && (base % ps != 0)) {
            // Round up to page boundary
            address = base + ps - (base & (ps - 1));
        } else {
            address = base;
        }
        cleaner = Cleaner.create(this, new Deallocator(base, size, cap));
        att = null;
    }
```




#### Direct mem GC
既然要调用System.gc，那肯定是想通过触发一次gc操作来回收堆外内存，不过我想先说的是堆外内存不会对gc造成什么影响(这里的System.gc除外)，但是堆外内存的回收其实依赖于我们的gc机制，首先我们要知道在java层面和我们在堆外分配的这块内存关联的只有与之关联的DirectByteBuffer对象了，它记录了这块内存的基地址以及大小，那么既然和gc也有关，那就是gc能通过操作DirectByteBuffer对象来间接操作对应的堆外内存了。DirectByteBuffer对象在创建的时候关联了一个PhantomReference，说到PhantomReference它其实主要是用来跟踪对象何时被回收的，它不能影响gc决策，但是gc过程中如果发现某个对象除了只有PhantomReference引用它之外，并没有其他的地方引用它了，那将会把这个引用放到java.lang.ref.Reference.pending队列里，在gc完毕的时候通知ReferenceHandler这个守护线程去执行一些后置处理，而DirectByteBuffer关联的PhantomReference是PhantomReference的一个子类，在最终的处理里会通过Unsafe的free接口来释放DirectByteBuffer对应的堆外内存块

对于System.gc的实现，**它会对新生代的老生代都会进行内存回收**，这样会比较彻底地回收DirectByteBuffer对象以及他们关联的堆外内存，我们dump内存发现DirectByteBuffer对象本身其实是很小的，但是它后面可能关联了一个非常大的堆外内存，因此我们通常称之为『冰山对象』，我们做ygc的时候会将新生代里的不可达的DirectByteBuffer对象及其堆外内存回收了，但是无法对old里的DirectByteBuffer对象及其堆外内存进行回收，这也是我们通常碰到的最大的问题，如果有大量的DirectByteBuffer对象移到了old，但是又一直没有做cms gc或者full gc，而只进行ygc，那么我们的物理内存可能被慢慢耗光，但是我们还不知道发生了什么，因为heap明明剩余的内存还很多(前提是我们禁用了System.gc)。


## 通过 Heapdump 查询 DirectByteBuffer
> http://www.mastertheboss.com/java/troubleshooting-outofmemoryerror-direct-buffer-memory/
> ```sql
> SELECT x, x.capacity FROM java.nio.DirectByteBuffer x WHERE ((x.capacity > 1024 * 1024) and (x.cleaner != null))
> ```

## Tunning

####  MaxDirectMemorySize 默认
CMS GC:

> 默认的 -XX:MaxDirectMemorySize = (新生代的最大值 - 一个survivor的大小) + 老生代的最大值

也就是：
> 默认的 -XX:MaxDirectMemorySize = 我们设置的-Xmx的值里 - 一个survivor的大小

但这里 http://www.mastertheboss.com/java/troubleshooting-outofmemoryerror-direct-buffer-memory/ 的表述不同：
> by digging into sun.misc.VM you will see that, if not configured, it derives its value from Runtime.getRuntime.maxMemory(), thus the value of –Xmx. So if you don’t configure -XX:MaxDirectMemorySize and do configure -Xmx2g, the “default” MaxDirectMemorySize will also be 2 Gb, and the total JVM memory usage of the app (heap+direct) may grow up to 2 + 2 = 4 Gb.

#### maxCachedBufferSize

Another option is to configure a limit per-thread DirectByteBuffer size using the -Djdk.nio.maxCachedBufferSize JVM property

`-Djdk.nio.maxCachedBufferSize`

The above JVM property will limit the per-thread DirectByteBuffer size.

> https://www.oracle.com/java/technologies/javase/8u102-relnotes.html
> 
> The system property jdk.nio.maxCachedBufferSize has been introduced in 8u102 to limit the memory used by the "temporary buffer cache." **The temporary buffer cache is a per-thread cache of direct memory used by the NIO implementation to support applications that do I/O with buffers backed by arrays in the java heap.** The value of the property is the maximum capacity of a direct buffer that can be cached. **If the property is not set, then no limit is put on the size of buffers that are cached.** Applications with certain patterns of I/O usage may benefit from using this property. In particular, an application that does I/O with large multi-megabyte buffers at startup but does I/O with small buffers may see a benefit to using this property. **Applications that do I/O using direct buffers will not see any benefit to using this system property**. 


> https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo
> 
> Setting `-Djdk.nio.maxCachedBufferSize` doesn't prevent allocation of a big `DirectByteBuffer` --- it only prevents this buffer from being cached and reused. So, if each of 10 threads allocates a 1GB `HeapByteBuffer`  and then invokes some I/O operation simultaneously, there will be a temporary RSS spike of up to 10GB, due to 10 temporary direct buffers. However, each of these `DirectByteBuffer`s will be deallocated immediately after the I/O operation. In contrast, any direct buffers smaller than the threshold will stick in memory until their owner thread terminates.
> If the JVM's RSS grows much higher than the maximum heap size, then, most likely, this is again a problem with `DirectByteBuffer`s. They may be created explicitly by the app (though this code may be in some third-party library), or they may be automatically created and cached by internal JDK code for I/O threads that use `HeapByteBuffer`s (some versions of Netty, for example, do that). Again, take a JVM heap dump and analyze it with JXRay. If there is a `java.lang.ThreadLocal`  in the reference chain that keeps the buffers in memory, refer to section 1 above; otherwise, return to section 2.

## Troubleshooting

### OutOfMemoryError: Direct Buffer Memory

> https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo
> 
> Internally, the JVM keeps track of the amount of native memory that is allocated and released by  DirectByteBuffers, and puts a limit on this amount. If your application fails with a stack trace like:
> ```
> Exception in thread … java.lang.OutOfMemoryError: Direct buffer memory
at java.nio.Bits.reserveMemory(Bits.java:694)
at java.nio.DirectByteBuffer.<init>(DirectByteBuffer.java:123)
at java.nio.ByteBuffer.allocateDirect(ByteBuffer.java:311)
...
> ```


### GLIBC
> https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo
> 
> It's known to occur in RedHat Enterprise Linux 6 (RHEL 6), running glibc memory allocator version 2.10 or newer on 64-bit machines.
> The `glibc` memory allocator, at least in some versions of Linux, has an optimization to improve speed by avoiding contention when a process has a large number of concurrent threads. The supposed speedup is achieved by maintaining per-core memory pools. Essentially, with this optimization, the OS grabs memory for a given process in pretty big same-size (64MB) chunks called arenas, which are clearly visible when process memory is analyzed with `pmap`. Each arena is available only to its respective CPU core, so no more than one thread at a time can operate on it. Then, individual `malloc()` calls reserve memory within these arenas. Up to a certain maximum number of arenas (8 by default) can be allocated per each CPU core. Looks like this is maxed out when the number of threads is high and/or threads are created and destroyed frequently. The actual amount of memory utilized by the application within these arenas can be quite small. However, if an application has a large number of threads, and the machine has a large number of CPU cores, the total amount of memory reserved in this way can grow really high. For example, on a machine with 16 cores, this number would be 16 * 8 * 64MB = 8GB.
> Fortunately, the maximum number of arenas can be adjusted via the `MALLOC_ARENA_MAX` environment variable. Because of this, some Java applications use a script like the one below to prevent this problem:
> ```bash
> # Some versions of glibc use an arena memory allocator that causes
> # virtual memory usage to explode. Tune the variable down to prevent
> # vmem explosion.
> export MALLOC_ARENA_MAX=${MALLOC_ARENA_MAX:-4}
> ```

### Native Memory Tracking (NMT)
> https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo
> one has to invoke the `jcmd` utility separately to obtain the information accumulated by the JVM so far. Both the detailed info (all the allocation events) or summary (how much memory is currently allocated by category, such as Class, Thread, Internal) is available. Memory allocated by `DirectByteBuffers`  is tracked under the **"Internal"** category.



## Ref
https://developer.aliyun.com/article/2948
https://www.evanjones.ca/java-bytebuffer-leak.html
https://dzone.com/articles/troubleshooting-problems-with-native-off-heap-memo

