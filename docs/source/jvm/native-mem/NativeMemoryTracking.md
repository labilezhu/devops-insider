

Example
```
Total:  reserved=664192KB,  committed=253120KB                                           <--- total memory tracked by Native Memory Tracking
 
-                 Java Heap (reserved=516096KB, committed=204800KB)                      <--- Java Heap
                            (mmap: reserved=516096KB, committed=204800KB)
 
-                     Class (reserved=6568KB, committed=4140KB)                          <--- class metadata
                            (classes #665)                                               <--- number of loaded classes
                            (malloc=424KB, #1000)                                        <--- malloc'd memory, #number of malloc
                            (mmap: reserved=6144KB, committed=3716KB)
 
-                    Thread (reserved=6868KB, committed=6868KB)
                            (thread #15)                                                 <--- number of threads
                            (stack: reserved=6780KB, committed=6780KB)                   <--- memory used by thread stacks
                            (malloc=27KB, #66)
                            (arena=61KB, #30)                                            <--- resource and handle areas
 
-                      Code (reserved=102414KB, committed=6314KB)
                            (malloc=2574KB, #74316)
                            (mmap: reserved=99840KB, committed=3740KB)
 
-                        GC (reserved=26154KB, committed=24938KB)
                            (malloc=486KB, #110)
                            (mmap: reserved=25668KB, committed=24452KB)
 
-                  Compiler (reserved=106KB, committed=106KB)
                            (malloc=7KB, #90)
                            (arena=99KB, #3)
 
-                  Internal (reserved=586KB, committed=554KB)
                            (malloc=554KB, #1677)
                            (mmap: reserved=32KB, committed=0KB)
 
-                    Symbol (reserved=906KB, committed=906KB)
                            (malloc=514KB, #2736)
                            (arena=392KB, #1)
 
-           Memory Tracking (reserved=3184KB, committed=3184KB)
                            (malloc=3184KB, #300)
 
-        Pooled Free Chunks (reserved=1276KB, committed=1276KB)
                            (malloc=1276KB)
 
-                   Unknown (reserved=33KB, committed=33KB)
                            (arena=33KB, #1)
```

> `direct byte buffer` allocations are counted to `Internal` section.


### What is `commited` memory

> https://docs.oracle.com/javase/7/docs/api/java/lang/management/MemoryUsage.html


A `MemoryUsage` object contains four values:

| `init` | represents the initial amount of memory (in bytes) that the Java virtual machine requests from the operating system for memory management during startup. The Java virtual machine may request additional memory from the operating system and may also release memory to the system over time. The value of `init` may be undefined. |
| --- |  --- |
| `used` | represents the amount of memory currently used (in bytes). |
| `committed` | represents the amount of memory (in bytes) that is guaranteed to be available for use by the Java virtual machine. The amount of committed memory may change over time (increase or decrease). The Java virtual machine may release memory to the system and `committed` could be less than `init`. `committed` will always be greater than or equal to `used`. |
| `max` | represents the maximum amount of memory (in bytes) that can be used for memory management. Its value may be undefined. The maximum amount of memory may change over time if defined. The amount of used and committed memory will always be less than or equal to `max` if `max` is defined. A memory allocation may fail if it attempts to increase the used memory such that `used > committed` even if `used <= max` would still be true (for example, when the system is low on virtual memory). |

## NMT Memory Categories

> https://docs.oracle.com/javase/8/docs/technotes/guides/troubleshoot/tooldescr022.html#BABCBGFA

| Category                 | Description                                                  |
| ------------------------ | ------------------------------------------------------------ |
| Java Heap                | The heap where your objects live                             |
| Class                    | Class meta data                                              |
| Code                     | Generated code                                               |
| GC                       | data use by the GC, such as card table                       |
| Compiler                 | Memory used by the compiler when generating code             |
| Symbol                   | Symbols                                                      |
| Memory Tracking          | Memory used by NMT itself                                    |
| Pooled Free Chunks       | Memory used by chunks in the arena chunk pool                |
| Shared space for classes | Memory mapped to class data sharing archive                  |
| Thread                   | Memory used by threads, including thread data structure, resource area and handle area and so on. |
| Thread stack             | Thread stack. It is marked as committed memory, but it might not be completely committed by the OS |
| Internal                 | Memory that does not fit the previous categories, such as the memory used by the command line parser, JVMTI, properties and so on. |
| Unknown                  | When memory category can not be determined.Arena: When arena is used as a stack or value objectVirtual Memory: When type information has not yet arrived |



## Ref

[Native Memory Tracking](https://docs.oracle.com/javase/8/docs/technotes/guides/troubleshoot/tooldescr007.html)

https://docs.oracle.com/javase/8/docs/technotes/guides/vm/nmt-8.html

https://gist.github.com/prasanthj/48e7063cac88eb396bc9961fb3149b58

https://support.cloudbees.com/hc/en-us/articles/360042655312-Diagnosing-Java-Native-Memory-Issues



https://support.cloudbees.com/hc/en-us/articles/360042655312-Diagnosing-Java-Native-Memory-Issues

https://picnic.app/careers/quest-to-the-os-java-native-memory

https://www.ibm.com/docs/en/support-assistant/5.0.0?topic=se-directbytebuffers

