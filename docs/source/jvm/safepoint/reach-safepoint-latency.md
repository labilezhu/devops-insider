# reach-safepoint-latency.md



## Cassandra and MMAP file

> from https://lists.apache.org/thread/rz8xynqv42109t6ybprhcmcp4n0mfg7j

Hello cassandra-users,

I'm investigating an issue with JVMs taking a while to reach a safepoint. I'd
like the list's input on confirming my hypothesis and finding mitigations.

My hypothesis is that slow block devices are causing Cassandra's JVM to pause
completely while attempting to reach a safepoint.

Background:

Hotspot occasionally performs maintenance tasks that necessitate stopping all
of its threads. Threads running JITed code occasionally read from a given
safepoint page. If Hotspot has initiated a safepoint, reading from that page
essentially catapults the thread into purgatory until the safepoint completes
(the mechanism behind this is pretty cool). Threads performing syscalls or
executing native code do this check upon their return into the JVM.

In this way, during the safepoint Hotspot can be sure that all of its threads
are either patiently waiting for safepoint completion or in a system call.

Cassandra makes heavy use of mmapped reads in normal operation. When doing
mmapped reads, the JVM executes userspace code to effect a read from a file. On
the fast path (when the page needed is already mapped into the process), this
instruction is very fast. When the page is not cached, the CPU triggers a page
fault and asks the OS to go fetch the page. The JVM doesn't even realize that
anything interesting is happening: to it, the thread is just executing a mov
instruction that happens to take a while.

The OS, meanwhile, puts the thread in question in the D state (assuming Linux,
here) and goes off to find the desired page. This may take microseconds, this
may take milliseconds, or it may take seconds (or longer). When I/O occurs
while the JVM is trying to enter a safepoint, every thread has to wait for the
laggard I/O to complete.

If you log safepoints with the right options [1], you can see these occurrences
in the JVM output:

If that safepoint happens to be a garbage collection (which this one was), you
can also see it in GC logs:

In this way, JVM safepoints become a powerful weapon for transmuting a single
thread's slow I/O into the entire JVM's lockup.

Does all of the above sound correct?

Mitigations:

1) don't tolerate block devices that are slow

This is easy in theory, and only somewhat difficult in practice. Tools like
perf and iosnoop [2] can do pretty good jobs of letting you know when a block
device is slow.

It is sad, though, because this makes running Cassandra on mixed hardware (e.g.
fast SSD and slow disks in a JBOD) quite unappetizing.

2) have fewer safepoints

Two of the biggest sources of safepoints are garbage collection and revocation
of biased locks. Evidence points toward biased locking being unhelpful for
Cassandra's purposes, so turning it off (-XX:-UseBiasedLocking) is a quick way
to eliminate one source of safepoints.

Garbage collection, on the other hand, is unavoidable. Running with increased
heap size would reduce GC frequency, at the cost of page cache. But sacrificing
page cache would increase page fault frequency, which is another thing we're
trying to avoid! I don't view this as a serious option.

3) use a different IO strategy

Looking at the Cassandra source code, there appears to be an un(der)documented
configuration parameter called disk_access_mode. It appears that changing this
to 'standard' would switch to using pread() and pwrite() for I/O, instead of
mmap. I imagine there would be a throughput penalty here for the case when
pages are in the disk cache.

Is this a serious option? It seems far too underdocumented to be thought of as
a contender.

4) modify the JVM

This is a longer term option. For the purposes of safepoints, perhaps the JVM
could treat reads from an mmapped file in the same way it treats threads that
are running JNI code. That is, the safepoint will proceed even though the
reading thread has not "joined in". Upon finishing its mmapped read, the
reading thread would test the safepoint page (check whether a safepoint is in
progress, in other words).

Conclusion:

I don't imagine there's an easy solution here. I plan to go ahead with
mitigation #1: "don't tolerate block devices that are slow", but I'd appreciate
any approach that doesn't require my hardware to be flawless all the time.

Josh



----



Even with memory mapped IO the kernel is going to do read ahead. It seems like if the issue is reading to much from the device it isn't going to help to use memory mapped files or smaller buffered reads. Maybe helps by some percentage, but it's still going to read quite a bit extra.



---

Well, you seem to be assuming: 

1) read ahead is done unconditionally, with an equal claim to disk resources 
2) read ahead is actually enabled (tuning recommendations are that it be disabled, or at least drastically reduced, to my knowledge) 
3) read ahead happens synchronously (even if you burn some bandwidth, not waiting the increased latency for all blocks means a faster turn around to client) Ignoring all of this, 64kb is 1/3 default read ahead in Linux, so you're talking a ~50% increase, which is not an amount I would readily dismiss.



---

So if the data is the page cache and you just access it regularly (sequentially) you get all the benefits of the prefetcher. If you go and touch every page first you will not have the latency of prefetching hidden from you.

When you fault a page the kernel has no idea how much you are going to read. If there is a mismatch then you may end up going back and forth to the device several times and for spinning disk this is worse. If you express up front what you want to read either by fadvise/madvise or a buffered read it can do something "smart". Granted IO scheduling ranges from middling to non-existent most of the time, and the fadvise/madvise stuff for this has holes I can't recall right now.





## Ref.
> https://www.evanjones.ca/jvm-mmap-pause-finding.html
