
> https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm

The JVM generates native code for a variety of reasons, including:
- for the dynamically generated interpreter loop
- Java Native Interface (JNI) stubs
- Java methods that are compiled into native code by the just-in-time (JIT) compiler.  **The JIT is by far the biggest user of the codecache.**


### How to Use the Codecache Options of the java Command

The options listed in the following sections share the following characteristics.

-   All options are `--XX` options, for example, `-XX:InitialCodeCacheSize=32m`. Options that have true/false values are specified using `+` for true and `-` for false. For example, `-XX:+PrintCodeCache` sets this option to true.

-   For any option that has "varies" listed as the default value, run the launcher with `XX:+PrintFlagsFinal` to see your platform's default value.

-   If the default value for an option differs depending on which JVM is being used (client or server), then both defaults are listed, separated by a '/'. The client JVM default is listed first. The minimal JVM uses the same JIT as the client JVM, and therefore has the same defaults.

### Codecache Size Options

[Table 15-1](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#BABBEJEF) summarizes the codecache size options. See also [Constraining the Codecache Size](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#A1100181).


Table 15-1 Codecache Size Options

| Option | Default | Description |
| :-- |  :-- |  :-- |
| `InitialCodeCacheSize` | 160K (varies) | Initial code cache size (in bytes) |
| `ReservedCodeCacheSize` | 32M/48M | Reserved code cache size (in bytes) - maximum code cache size |
| `CodeCacheExpansionSize` | 32K/64K | Code cache expansion size (in bytes) |


### Codecache Flush Options

[Table 15-2](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#BABBACEJ) summarizes the codecache flush options.


Table 15-2 Codecache Flush Options

| Option | Default | Description |
| :-- |  :-- |  :-- |
| `ExitOnFullCodeCache` | false | Exit the JVM if the codecache fills |
| `UseCodeCacheFlushing` | false | Attempt to sweep the codecache before shutting off compiler |
| `MinCodeCacheFlushingInterval` | 30 | Minimum number of seconds between codecache sweeping sessions |
| `CodeCacheMinimumFreeSpace` | 500K | When less than the specified amount of space remains, stop compiling. This space is reserved for code that is not compiled methods, for example, native adapters. |


### Compilation Policy Options

[Table 15-3](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#BABDBEFF) summarizes the compilation policy (when to compile) options.


Table 15-3 Compilation Policy Options

| Option | Default | Description |
| :-- |  :-- |  :-- |
| `CompileThreshold` | 1000 or 1500/10000 | Number of interpreted method invocations before (re-)compiling |
| `OnStackReplacePercentage` | 140 to 933 | NON_TIERED number of method invocations/branches (expressed as a percentage of `CompileThreshold`) before (re-)compiling OSR code |


### Compilation Limit Options

[Table 15-4](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#BABGGHJE) summarizes the compilation limit options, which determine how much code is compiled).


Table 15-4 Compilation Limit Options

| Option | Default | Description |
| :-- |  :-- |  :-- |
| `MaxInlineLevel` | 9 | Maximum number of nested calls that are inlined |
| `MaxInlineSize` | 35 | Maximum bytecode size of a method to be inlined |
| `MinInliningThreshold` | 250 | Minimum invocation count a method needs to have to be inlined |
| `InlineSynchronizedMethods` | true | Inline synchronized methods |


### Diagnostic Options

[Table 15-5](https://docs.oracle.com/javase/8/embedded/develop-apps-platforms/codecache.htm#BABDCEDJ) summarizes the diagnostic options.


Table 15-5 Diagnostic Options

| Option | Default | Description |
| :-- |  :-- |  :-- |
| `PrintFlagsFinal` | false | Print all JVM options after argument and ergonomic processing |
| `PrintCodeCache` | false | Print the code cache memory usage when exiting |
| `PrintCodeCacheOnCompilation` | false | Print the code cache memory usage each time a method is compiled |


### Constraining the Codecache Size

Constraining the codecache size means the codecache is limited to a size that is less than what would an unconstrained codecache would use. The `ReservedCodeCacheSize` option determines the maximum size of the codecache. It defaults to a minimum of 32MB for the client JVM and 48MB for the server VM. For most Java applications, this size is so large that the application will never fill the entire codecache. Thus the codecache is viewed as being unconstrained, **meaning the JIT will continue to compile any code that it thinks should be compiled**.

### When is Constraining the Codecache Size Useful?

**Applications that make state changes that result in a new set of methods being "hot" can benefit greatly from a constrained codecache.**

A common state change is from startup to regular execution. The application might trigger a lot of compilation during startup, but very little of this compiled code is needed after startup. By constraining the codecache, you will trigger codecache flushing to throw away the code compiled during startup to make room for the code needed during application execution.

### How to Constrain the Codecache Size

The `UseCodeCacheFlushing` option turns codecache flushing on and off. By default it is on. You can disable this feature by specifying `XX:-UseCodeCacheFlushing`. When enabled, the codecache flushing is triggered when the memory available in the codecache is low. It is critical to enable codecache flushing if you constrain the codecache. If flushing is disabled, the JIT does not compile methods after the codecache fills up.


### Reducing Compilations
Two main command line options that affect how aggressively methods are compiled: `CompileThreshold` and `OnStackReplacePercentage`. `CompileThreshold` relates to the number of method invocations needed before the method is compiled. `OnStackReplacePercentage` relates to the number of backwards branches taken in a method before it gets compiled, and is specified as a percentage of `CompileThreshold`. 

When a method's combined number of backwards branches and invocations reaches or exceeds `CompileThreshold` * `OnStackReplacePercentage` / 100, the method is compiled. Note that there is also an option called `BackEdgeThreshold`, but it currently does nothing. Use `OnStackReplacePercentage` instead.


### CodeCache size
> https://julio-falbo.medium.com/understand-jvm-and-jit-compiler-part-4-9738194ad06e

As always, the JVM will provide us a message in the console of our application telling us that the CodeCache is full. The message is:

```log
VM warning: CodeCache is full. The compiler has been disabled.
```

We can easily see information about our Code Cache enabling a JVM option called: **-XX:+PrintCodeCache**.We can easily see information about our Code Cache enabling a JVM option called: `-XX:+PrintCodeCache`.

if you are running with **Java8-,** the output will be similar to this: *(I will explain this difference later in this same article.)*

```
CodeCache: size=245760Kb used=1149Kb max_used=1162Kb free=244610Kb
 bounds [0x000000011489d000, 0x0000000114b0d000, 0x000000012389d000]
 total_blobs=298 nmethods=40 adapters=172
 compilation: enabled
```

If you are using **Java 8 or above** then the max size will be **240 megabytes** but if you disable tiered compilation with the option **-XX:-TieredCompilation**, then the default size is **48 megabytes**.

#### CodeCache Java8 vs Java9+

Basically, in **Java 9** we had the **JEP (JDK Enhancement Proposal) 197** that was responsible to split the Code Cache into 3 different segments (also called Code Heaps), "**non-method", "profiled"** and **"non-profiled".**

The "**non-method**" code heap contains non-method code such as compiler buffers and bytecode interpreters. This code type stays in the code cache forever. The code heap has a fixed size of 3 MB and the remaining code cache is distributed evenly among the profiled and non-profiled code heaps.

The "**profiled**" code heap contains lightly optimized, profiled methods with a short lifetime.

And the "**non-profiled**" code heap contains fully optimized, non-profiled methods with a potentially long lifetime.

> but why did they do that?

According to the official page of the JEP 197, the goals are:

-   Separate non-method, profiled, and non-profiled code
-   Shorter sweep times due to specialized iterators that skip non-method code
-   Improve execution time for some compilation-intensive benchmarks
-   Better control of JVM memory footprint
-   Decrease fragmentation of highly-optimized code
-   Improve code locality because code of the same type is likely to be accessed close in time
-   Better iTLB and iCache behavior
-   Establish a base for future extensions
-   Improved management of heterogeneous code; for example, Sumatra (GPU code) and AOT compiled code
-   Possibility of fine-grained locking per code heap
-   Future separation of code and metadata (see [JDK-7072317](https://bugs.openjdk.java.net/browse/JDK-7072317))

