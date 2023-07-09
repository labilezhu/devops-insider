

> https://docs.oracle.com/javase/7/docs/api/java/lang/management/MemoryMXBean.html

public interface MemoryMXBean
extends PlatformManagedObject

The management interface for the memory system of the Java virtual machine.

A Java virtual machine has a single instance of the implementation class of this interface. This instance implementing this interface is an MXBean that can be obtained by calling the ManagementFactory.getMemoryMXBean() method or from the platform MBeanServer method.

The ObjectName for uniquely identifying the MXBean for the memory system within an MBeanServer is:

    java.lang:type=Memory 

It can be obtained by calling the PlatformManagedObject.getObjectName() method. 

 The memory system of the Java virtual machine manages the following kinds of memory: 

  1. Heap 


 2. Non-Heap Memory 

The Java virtual machine manages memory other than the heap (referred as non-heap memory).


- **(Metaspace)**

The Java virtual machine has a method area that is shared among all threads. The method area belongs to non-heap memory. It stores per-class structures such as a runtime constant pool, field and method data, and the code for methods and constructors. It is created at the Java virtual machine start-up.

The method area is logically part of the heap but a Java virtual machine implementation may choose not to either garbage collect or compact it. Similar to the heap, the method area may be of a fixed size or may be expanded and shrunk. The memory for the method area does not need to be contiguous.

- **Codecache**

In addition to the method area, a Java virtual machine implementation may require memory for internal processing or optimization which also belongs to non-heap memory. For example, the JIT compiler requires memory for storing the native machine code translated from the Java virtual machine code for high performance. 