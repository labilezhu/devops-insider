# Use of MappedByteBuffer could cause long STW due to TTSP (time to safepoint)

> https://ocadaruma.hatenablog.com/entry/2022/12/25/175333



In [JVM](http://d.hatena.ne.jp/keyword/JVM), "safepoint" is a point that all threads are suspended so we can get the consistent view of the thread states.

[openjdk.org](https://openjdk.org/groups/hotspot/docs/HotSpotGlossary.html)

[JVM](http://d.hatena.ne.jp/keyword/JVM) executes operations that needs to be done inside safepoint (e.g. thread dump) in below procedure:

- 1 Call `SafepointSynchronize::begin()`
  - 1-1 [Per-thread "polling page" address is set to "armed" value](https://github.com/adoptium/jdk11u/blob/jdk-11.0.17%2B8/src/hotspot/share/runtime/safepointMechanism.inline.hpp#L66)
  - 1-2 Every thread voluntarily brings itself to safepoint
- 2 Wait all threads reach safepoint
- 3 Execute the operation
- 4 Finish safepoint and resumes all threads by calling [`SafepointSynchronize::end()`](https://github.com/adoptium/jdk11u/blob/jdk-11.0.17%2B8/src/hotspot/share/runtime/safepoint.cpp#L499)

The duration from 1 to 3 is called "time to safepoint" (TTSP).

refs:[krzysztofslusarski.github.io](https://krzysztofslusarski.github.io/2020/11/13/stw.html)

Since entire application can't make progress after `SafepointSynchronize::begin()` until the safepoint ends, in the meantime, the application is called in STW (stop the world) which is known to cause latency issues in [JVM](http://d.hatena.ne.jp/keyword/JVM).

Clearly, if the operation itself takes long time (e.g. full [GC](http://d.hatena.ne.jp/keyword/GC)) it leads to long STW.

However, it's known that TTSP also could be long sometimes, which also leads to long STW.

If you enable [JVM](http://d.hatena.ne.jp/keyword/JVM) logging with `-Xlog:safepoint=info` or verbose, you can see the logs like blow:

```
[info][safepoint] Total time for which application threads were stopped: 0.0001794 seconds, Stopping threads took: 0.0000587 seconds
```

`Total time for which application threads were stopped` indicates total STW duration and `Stopping threads took` is the TTSP duration.

## How threads reach safepoint

Threads reach safepoint in several ways depending on the executed code.

There's a very good article on the web about this:

[psy-lob-saw.blogspot.com](http://psy-lob-saw.blogspot.com/2015/12/safepoints.html)

In short,

- If a thread is being blocked for lock etc, the thread is assumed at safepoint
- If a thread is executing native code, the thread is assumed at safepoint
- If a thread is executing bytecode, threads check polling page at "reasonable interval" and enters safepoint in next check
  - The above article reproduces long TTSP by exploiting this, by doing large iteration loop that no polling page check is inserted (called "counted loop")

Hence, if single, non-native instruction could take long time, it naturally causes long TTSP because the thread can't reach safepoint in the meantime.

File-backed MappedByteBuffer (Memory mapped file) is an example of such case.

With MappedByteBuffer, we can map the file on the (possibly slow) device to the virtual memory. (which uses `mmap` system call internally)

When we call `MappedByteBuffer#get`, if the file content is not loaded to physical memory (e.g. because the mapping is newly created or already evicted from page cache), it causes "major page fault" which involves reading the file, possibly slow when the file is located on a spinning drive.

If a drive that hosts the file about to be mapped got broken (it's not uncommon when we run a middleware that needs huge storage space so many HDDs are attached), it easily causes hundreds ~ tens of thousands of milliseconds page fault duration, and if a safepoint is initiated unluckily, the entire application will be stopped in the meantime and it's a serious functionality issue.

## Experiment

Let's confirm that the long major page fault actually causes long TTSP by simple [Java program](https://gist.github.com/ocadaruma/c459addbacf78550261fc94a3bdda916), which just instantiates MappedByteBuffer from the given path and tries read from it.

To reproduce faulty disk, we use [ddi](https://github.com/kawamuray/ddi) to inject delays to the device.

Result:

```
# - Assume vmtouch is installed
# - Assume a file is prepared in /data_xfs/test/test.dat
# - Assume ddi is setup on /data_xfs

# Inject 5 seconds read delay into the device
$ echo 5000 | sudo tee /sys/fs/ddi/252\:18/read_delay

# Evict the file from page cache
$ vmtouch -e /data_xfs/test/test.dat

# Run a program
$ java -Xlog:safepoint=info:file=jvm.log LongTTSP /data_xfs/test/test.dat
[2022-12-25T08:35:34.156032] (main) pid: 33246
[2022-12-25T08:35:34.157010] (main) Sleeping...
[2022-12-25T08:35:34.157051] (reader-thread) Gonna read from mapped file
[2022-12-25T08:35:39.201298] (reader-thread) Finished read in 5043418 us

# In another terminal, take jstack to initiate the safepoint right after the application started
$ jstack 33246

# Check the JVM log
$ grep stopped jvm.log | grep -v 'stopped: 0'
[5.118s][info][safepoint] Total time for which application threads were stopped: 4.0402532 seconds, Stopping threads took: 4.0391063 seconds
```

As we can see, the application got STW for 4 seconds and the [most](http://d.hatena.ne.jp/keyword/most) part was for TTSP.

## Conclusion

- MappedByteBuffer could cause long STW with slow device for long major page fault
  - We have to consider carefully if memory mapped file is actually necessary
  - Or we have to a mechanism to "warm up" the memory map, to ensure the file is loaded before accessing it through MappedByteBuffer
    - In fact, I found there's a [discussion thread in cassandra ML](https://lists.apache.org/thread/rz8xynqv42109t6ybprhcmcp4n0mfg7j) and seems it mentioned similar idea
