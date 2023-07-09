


HotSpot JVM has a feature called CompressedOops with which it uses 32-bit (compressed) pointers on 64-bit platforms to have smaller footprints and better performance on 64-bit platforms. 64-bit address values are stored into 32-bit pointers using an encoding and are retrieved back using the corresponding decoding.

CompressedOops implementation tries to allocate Java Heap using different strategies based on the heap size and the platform it runs on. If the heap size is less than 4Gb then it tries to allocate it into low virtual address space (below 4Gb) so that the compressed oops can be used without encoding/decoding. If it fails or heap size > 4Gb then it tries to allocate the heap below 32Gb to use zero based compressed oops. If this also fails then it switches to regular compressed oops with narrow oop base.



## Ref
https://blogs.oracle.com/poonam/running-on-a-64bit-platform-and-still-running-out-of-memory
[JVM Anatomy Quark #23: Compressed References](https://shipilev.net/jvm/anatomy-quarks/23-compressed-references).
https://stuefe.de/posts/metaspace/what-is-compressed-class-space/