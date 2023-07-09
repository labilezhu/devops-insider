

## hsperfdata

```bash
strace -f  -e trace=file jps

[pid 60841] newfstatat(AT_FDCWD, "/tmp", {st_mode=S_IFDIR|S_ISVTX|0777, st_size=4096, ...}, 0) = 0
[pid 60841] openat(AT_FDCWD, "/tmp", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 5
[pid 60841] newfstatat(5, "", {st_mode=S_IFDIR|S_ISVTX|0777, st_size=4096, ...}, AT_EMPTY_PATH) = 0
[pid 60841] newfstatat(AT_FDCWD, "/tmp/hsperfdata_labile", {st_mode=S_IFDIR|0755, st_size=4096, ...}, 0) = 0
[pid 60841] openat(AT_FDCWD, "/tmp/hsperfdata_labile", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 5
[pid 60841] newfstatat(5, "", {st_mode=S_IFDIR|0755, st_size=4096, ...}, AT_EMPTY_PATH) = 0
[pid 60841] newfstatat(AT_FDCWD, "/tmp/hsperfdata_labile/59332", {st_mode=S_IFREG|0600, st_size=32768, ...}, 0) = 0
[pid 60841] faccessat(AT_FDCWD, "/tmp/hsperfdata_labile/59332", R_OK) = 0
[pid 60841] newfstatat(AT_FDCWD, "/tmp/hsperfdata_labile/60840", {st_mode=S_IFREG|0600, st_size=32768, ...}, 0) = 0
[pid 60841] faccessat(AT_FDCWD, "/tmp/hsperfdata_labile/60840", R_OK) = 0
[pid 60841] openat(AT_FDCWD, "/tmp", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 5
[pid 60841] newfstatat(5, "", {st_mode=S_IFDIR|S_ISVTX|0777, st_size=4096, ...}, AT_EMPTY_PATH) = 0


```

File:
/tmp/hsperfdata_$USERNAME/$PID


```log
bash-4.4$ ls -l /proc/`pgrep java`/map_files | grep -v so | grep -v jar
total 0
lrw------- 1 webapp webapp 64 Sep 30 03:07 7f9ced63d000-7f9ced645000 -> /tmp/hsperfdata_webapp/111


ls -l /proc/`pgrep java`/fd | grep -v so | grep -v jar
```


Disable gathering those stats via a JVM option called `-XX:-UsePerfData` 

> https://www.evanjones.ca/jvm-mmap-pause.html

The JVM by default exports statistics by `mmap`-ing a file in `/tmp` (hsperfdata). On Linux, modifying a memory mapped file can block until disk I/O completes, which can be hundreds of milliseconds. Since the JVM modifies these statistics during garbage collection and safepoints, this causes pauses that are hundreds of milliseconds long. To reduce worst-case pause latencies, add the `-XX:+PerfDisableSharedMem` JVM flag to disable this feature. This will break tools that read this file, like `jstat`. **Update**: [how I found this problem](https://www.evanjones.ca/jvm-mmap-pause-finding.html)

GC log IO blocking:

> https://www.evanjones.ca/jvm-mmap-pause-finding.html

To get an idea of how the temporary directory gets combined with the user name, browse the Hotspot sources at https://openjdk.dev.java.net/hotspot and OpenGrok 'get_user_tmp_dir' or 'hsperfdata'. 


-XX:+UsePerfData

    Enables the perfdata feature. This option is enabled by default to allow JVM monitoring and performance testing. Disabling it suppresses the creation of the hsperfdata_userid directories. To disable the perfdata feature, specify -XX:-UsePerfData.

-XX:+PerfDisableSharedMem : 

    option forces JVM to use anonymous memory for Performance Counters instead of a mapped file. This helps to avoid random VM pauses caused by spontaneous disk I/O.

> https://www.tutorialguruji.com/java/is-there-any-performance-downsides-to-using-the-xxperfdisablesharedmem-jvm-flag/


> http://xmlandmore.blogspot.com/2013/09/hotspot-using-jstat-to-explore.html 


HotSpot provides jvmstat instrumentation for performance testing and problem isolation purposes.  And it's enabled by default (see -XX:+UsePerfData).

If you run Java application benchmarks, it's also useful to save PerfData memory to hsperfdata_<pid> file on exit by setting:</pid>

-   -XX:+PerfDataSaveToFile

A file named  hsperfdata_<vmid> will be saved in the WebLogic domain's top-level folder.

### How to Read hsperfdata File?

To display statistics collected in PerfData memory, you can use:

-   jstat^[3]^

-   Experimental JVM Statistics Monitoring Tool - It can attach to an instrumented HotSpot Java virtual machine and collects and logs performance statistics as specified by the command line options. (formerly jvmstat)

There are two ways of showing statistics collected in PerfData memory:

-   Online

-   You can attach to an instrumented HotSpot JVM and collect and log performance statistics at runtime.

-   Offline

-   You can set -XX:+PerfDataSaveToFile flag and read the contents of the hsperfdata_<pid> file on the exit of JVM.</pid>

In the following, we have shown an offline example of reading the hsperfdata_<pid> file (i.e. a binary file; you need to use *jstat*^[3]^ to display its content):</pid>$ /scratch/perfgrp/JVMs/jdk-hs/bin/jstat -class file:///<Path to Domain>/MyDomain/hsperfdata_9872
  <u>Loaded</u>    <u>Bytes</u>  <u>Unloaded</u>   <u>Bytes</u>       <u>Time</u>
30600   64816.3         2     3.2      19.74

You can check all available command options supported by jstat using:

$jdk-hs/bin/jstat -options
-class
-compiler
-gc
-gccapacity
-gccause
-gcmetacapacity
-gcnew
-gcnewcapacity
-gcold
-gcoldcapacity
-gcutil
-printcompilation

### HotSpot Just-In-Time Compiler Statistics

One of the command option supported by jstat is "-compiler", which can provide high-level JIT compiler statistics.

| <u>Column</u> | <u>Description</u> |
| :-- |  :-- |
| **Compiled** | Number of compilation tasks performed. |
| **Failed** | Number of compilation tasks that failed. |
| **Invalid** | Number of compilation tasks that were invalidated. |
| **Time** | Time spent performing compilation tasks. |
| **FailedType** | Compile type of the last failed compilation. |
| **FailedMethod** | Class name and method for the last failed compilation. |

In the following, we have shown the compiler statistics of three managed servers in one WLS Domain using two different JVM builds:

$/scratch/perfgrp/JVMs/jdk-hs/bin/jstat -compiler file:///<Path to Domain>/MyDomain/hsperfdata_9872

**<u>JVM1</u>**
**<u>Compiled</u> <u>Failed</u> <u>Invalid</u>   <u>Time</u>   <u>FailedType</u> <u>FailedMethod</u>**
   33210     13       0   232.97          1 oracle/ias/cache/Bucket objInvalidate
   74054     20       0   973.03          1 oracle/security/o5logon/b b
   74600     18       0  1094.21          1 oracle/security/o5logon/b b
  **<u>JVM2</u>**
  **<u>Compiled</u> <u>Failed</u> <u>Invalid</u>   <u>Time</u>   <u>FailedType</u> <u>FailedMethod</u>**
   33287     10       0   246.26          1 oracle/ias/cache/Bucket objInvalidate
   68237     18       0  1022.46          1 oracle/security/o5logon/b b
   67346     18       0   943.79          1 oracle/security/o5logon/b b

Given the above statistics, we could take next action on analyzing why JVM2 generating less compiled methonds than JVM1 did. At least this is one of the use case for using PerfData with its associated tool---jstat.

### PerfData-Related JVM Options

| Name | Description | Default | Type |
| --- |  --- |  --- |  --- |
| UsePerfData | Flag to disable jvmstat instrumentation for performance testing and problem isolation purposes. | true | bool |
| [](http://stas-blogspot.blogspot.com/2011/07/most-complete-list-of-xx-options-for.html)PerfDataSaveToFile | Save PerfData memory to hsperfdata_<pid> file on exit</pid> | false | bool |
| [](http://stas-blogspot.blogspot.com/2011/07/most-complete-list-of-xx-options-for.html)PerfDataSamplingInterval | Data sampling interval in milliseconds | 50 /*ms*/ | intx |
| [](http://stas-blogspot.blogspot.com/2011/07/most-complete-list-of-xx-options-for.html)PerfDisableSharedMem | Store performance data in standard memory | false | bool |
| [](http://stas-blogspot.blogspot.com/2011/07/most-complete-list-of-xx-options-for.html)PerfDataMemorySize | Size of performance data memory region. Will be rounded up to a multiple of the native os page size. | 32*K | intx |

Note that the default size of PerfData memory is 32K. Therefore the file (i.e., hsperfdata_ file) dumped on exit is also 32K in size.





> http://openjdk.java.net/groups/hotspot/docs/Serviceability.html#bjvmstat

<a name="bjvmstat" id="bjvmstat">HotSpot Jvmstat Performance Counters</a>
-------------------------------------------------------------------------

The HotSpot JVM exports a set of instrumentation objects, or counters as they are typically called. The counters are always on and so are updated by HotSpot in such a way as to impose minimal overhead to the running application. The set of counters exported by a JVM is not static as a JVM may create certain counters only when appropriate arguments are specified on the command line. Furthermore, different versions of a JVM may export very different sets of instrumentation. The counters have structured names such as *sun.gc.generation.1.name, java.threads.live, java.cls.loadedClasses*. The names of these counters and the data structures used to represent them are considered private, uncommitted interfaces to the HotSpot JVM. Users should not become dependent on any counter names, particularly those that start with prefixes other than "java.".

These counters are exposed to observers in different processes by means of a shared memory file. This allows observers in other processes to poll the counters without imposing any overhead on the target process. The java.io.tmpdir system property contains the pathname of the directory in which this file resides. The Solaris and Linux shared memory implementations use the mmap interface with a backing store file to implement named shared memory. Using the file system as the name space for shared memory allows a common name space to be supported across a variety of platforms. It also provides a name space that Java applications can deal with through simple file APIs. The Solaris and Linux implementations store the backing store file in a user specific temporary directory located in the /tmp file system, which is always a local file system and is sometimes a RAM based file system. The name of the file is:

> /tmp/hsperfdata_*user-name*/*vm-id*
