---
date: 2022-06-29T23:12:15+08:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
description: 
tags:
- performance
- hardware
---

# Low latency tuning

> [Low latency tuning guide](https://rigtorp.se/low-latency-guide/)

# Low latency tuning guide

2020-03-25

This guide describes how to tune your AMD64/x86_64 hardware and Linux system for running real-time or low latency workloads. Example workloads where this type of tuning would be appropriate:

- Line rate [packet capture](https://en.wikipedia.org/wiki/Packet_analyzer)
- Line rate [deep packet inspection (DPI)](https://en.wikipedia.org/wiki/Deep_packet_inspection)
- Applications using kernel-bypass networking
- Accurate benchmarking of [CPU bound](https://stackoverflow.com/questions/868568/what-do-the-terms-cpu-bound-and-i-o-bound-mean) programs

The term latency in this context refers to the time between receiving some event and the time when the event was processed. For example:

- The time between a network packet was received by a NIC until an application finished processing the packet.
- The time between a request was submitted to a queue and the worker thread finished processing the request.

To achieve low latency this guide describes how to:

- Maximize per core performance by maximizing CPU frequency and disabling power saving features.
- Minimize jitter caused by interrupts, timers and other applications interfering with your workload.

You can measure the reduced system jitter using my tool [hiccups](https://github.com/rigtorp/hiccups). In this example it shows how core 3 was isolated and experienced a maximum jitter of 18 us:

```console
$ hiccups | column -t -R 1,2,3,4,5,6
cpu  threshold_ns  hiccups  pct99_ns  pct999_ns    max_ns
  0           168    17110     83697    6590444  17010845
  1           168     9929    169555    5787333   9517076
  2           168    20728     73359    6008866  16008460
  3           168    28336      1354       4870     17869
```

[Discussion on HN](https://news.ycombinator.com/item?id=24027366).

## Hardware tuning

### Enable performance mode

The systems UEFI or BIOS usually have a setting for energy profile that adjusts available CPU power states, you should set this to “maximum performance” or equivalent.

### Disable hyper-threading

[Hyper-threading (HT)](https://en.wikipedia.org/wiki/Hyper-threading) or [Simultaneous multithreading (SMT)](https://en.wikipedia.org/wiki/Hyper-threading) is a technology to maximize processor resource usage for workloads with low [instructions per cycle (IPC)](https://en.wikipedia.org/wiki/Instructions_per_cycle). Since HT/SMT increases contention on processor resources it’s recommended to turn it off if you want to reduce jitter introduced by contention on processor resources. Disabling HT / SMT has the additional benefit of doubling (in case of 2-way SMT) the effective L1 and L2 cache available to a thread.

There are multiple methods to disable SMT / HT:

- In your system’s UEFI or BIOS firmware settings. This is the method I recommend.

- At boot time using the kernel command line parameter [`nosmt`](https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html?highlight=nosmt).

- At runtime using [SMT Control](https://www.kernel.org/doc/html/latest/admin-guide/hw-vuln/l1tf.html#smt-control):

  ```console
  echo off > /sys/devices/system/cpu/smt/control
  ```

- Using the [CPU hot-plugging functionality](https://www.kernel.org/doc/html/latest/core-api/cpu_hotplug.html) to disable one of a pair of sibling threads. Use `lscpu --extended` or `cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list` to determine which “CPUs” are sibling threads.

To verify that SMT / HT is disabled the output of the following command should be 0:

```console
cat /sys/devices/system/cpu/smt/active
0
```

References:

- “Simultaneous Multithreading in Red Hat Enterprise Linux”. https://access.redhat.com/solutions/rhel-smt

### Enable Turbo Boost

[Intel Turbo Boost](https://en.wikipedia.org/wiki/Intel_Turbo_Boost) and [AMD Turbo Core](https://en.wikipedia.org/wiki/AMD_Turbo_Core) technologies allows the processor to automatically overclock itself as long as it stays within some power and thermal envelope. If you have good cooling (set fan speed to max in BIOS), disable unused cores in BIOS or using the CPU hotplug functionality it’s possible to run your application cores continuously at the higher boost frequency.

Check if turbo boost is enabled:

```console
cat /sys/devices/system/cpu/intel_pstate/no_turbo
```

Output should be 0 if turbo boost is enabled.

Alternatively you can use `cpupower` to check the status of turbo boost:

```console
cpupower frequency-info
```

Use the `turbostat` tool to verify the clock frequency of each core. Note that `turbostat` will cause scheduling jitter and should not be used during production.

References:

- https://www.kernel.org/doc/html/latest/admin-guide/pm/cpufreq.html#frequency-boost-support
- https://www.kernel.org/doc/html/latest/core-api/cpu_hotplug.html

### Overclocking

Consider overclocking your processors. Running your processor at higher frequency will reduce jitter and latency. It’s not possible to overclock Intel Xeon server processors, but you can overclock Intel’s consumer gaming processors and AMD’s processors.

## Kernel tuning

### Use the performance CPU frequency scaling governor

Use the performance CPU frequency scaling governor to maximize core frequency.

Set all cores to use the performance governor:

```console
# find /sys/devices/system/cpu -name scaling_governor -exec sh -c 'echo performance > {}' ';'
```

This can also be done by using the `tuned` performance profile:

```console
# tuned-adm profile latency-performance
```

Verify that the performance governor is used with `cpupower`:

```console
cpupower frequency-info
```

References:

- https://www.kernel.org/doc/html/latest/admin-guide/pm/cpufreq.html
- https://www.kernel.org/doc/html/latest/admin-guide/pm/intel_pstate.html
- https://github.com/redhat-performance/tuned

### Isolate cores

By default the kernel scheduler will load balance all threads across all available cores. To stop system threads from interfering with your application threads from you can use the kernel command line option `isolcpus`. It disables scheduler load balancing for the isolated cores and causes threads to be restricted to the non-isolated cores by default. Note that your critical application threads needs to be specifically pinned to the isolated cores in order to run there.

For example to isolate cores 1 through 7 add `isolcpus=1-7` to your kernel command line.

When using isolcpus the kernel will still create several kernel threads on the isolated cores. Some of these kernel threads can be moved to the non-isolated cores.

Try to move all kernel threads to core 0:

```console
# pgrep -P 2 | xargs -i taskset -p -c 0 {}
```

Alternatively use `tuna` move all kernel threads away from cores 1-7:

```console
# tuna --cpus=1-7 --isolate
```

Verify by using the `tuna` command to show CPU affinities for all threads:

```console
tuna -P
```

Additionally kernel workqueues needs to be moved away from isolated cores. To move all work queues to core 0 (cpumask 0x1):

```console
# find /sys/devices/virtual/workqueue -name cpumask  -exec sh -c 'echo 1 > {}' ';'
```

Verify by listing current workqueue affinities:

```console
find /sys/devices/virtual/workqueue -name cpumask -print -exec cat '{}' ';'
```

Finally verify if cores were successfully isolated by checking how many thread context switches are occurring per core:

```console
# perf stat -e 'sched:sched_switch' -a -A --timeout 10000
```

The isolated cores should show a very low context switch count.

There is a work in progress patch set to improve task isolation even further, see [A full task-isolation mode for the kernel](https://lwn.net/Articles/816298/).

References:

- https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html?highlight=isolcpus
- https://www.kernel.org/doc/html/latest/admin-guide/kernel-per-CPU-kthreads.html
- http://man7.org/linux/man-pages/man2/sched_setaffinity.2.html
- https://rt.wiki.kernel.org/index.php/Tuna
- https://lwn.net/Articles/659490/
- https://lwn.net/Articles/816298/
- https://lwn.net/ml/linux-kernel/aed12dd15ea2981bc9554cfa8b5e273c1342c756.camel@marvell.com/

### Reducing timer tick interrupts

The scheduler runs regularly on each core in order to switch between runnable threads. This will introduce jitter for latency critical applications. If you have isolated your application cores and are running a single application thread per isolated core you can use the [`nohz_full`](https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html?highlight=nohz_full) kernel command line option in order to suppress the timer interrupts.

For example to enable `nohz_full` on cores 1-7 add `nohz_full=1-7 rcu_nocbs=1-7` to your kernel command line.

It’s important to note that the timer tick is only disabled on a core when there is only a single runnable thread scheduled on that core. You can see the number of runnable threads per core in */proc/sched_debug*.

The virtual memory subsystem runs a per core statistics update task every 1 second by default. You can reduce this interval by setting `vm.stat_interval` to a higher value, for example 120 seconds:

```console
# sysctl vm.stat_interval=120
```

Finally you can verify that the timer interrupt frequency is reduced by inspecting */proc/interrupts* or using [*perf*](https://perf.wiki.kernel.org/index.php/Main_Page) to monitor timer interrupts:

```console
# perf stat -e 'irq_vectors:local_timer_entry' -a -A --timeout 30000

 Performance counter stats for 'system wide':

CPU0                   31,204      irq_vectors:local_timer_entry
CPU1                    3,771      irq_vectors:local_timer_entry
CPU2                        3      irq_vectors:local_timer_entry
CPU3                        4      irq_vectors:local_timer_entry

      30.001671482 seconds time elapsed
```

In the above example cores 2 and 3 has a reduced timer interrupt frequency. Expect `isolcpus` + `nohz_full` cores to show a timer interrupt every other second or so. Unfortunately the timer tick cannot be completely eliminated.

References:

- The Linux Kernel Documentation. “NO_HZ: Reducing Scheduling-Clock Ticks”. https://www.kernel.org/doc/html/latest/timers/no_hz.html
- Jonathan Corbet. “(Nearly) full tickless operation in 3.10 “. https://lwn.net/Articles/549580/
- Jeremy Eder. “nohz_full=godmode ?”. https://jeremyeder.com/2013/11/15/nohz_fullgodmode/

### Interrupt affinity

Reduce jitter from interrupt processing by changing the CPU affinity of the interrupts. This can easily be done by running [*irqbalance*](https://github.com/Irqbalance/irqbalance). By default *irqbalance* will automatically isolate the cores specified by the kernel command line parameter `isolcpus`. You can also specify cores to isolate using the `IRQBALANCE_BANNED_CPUS` environment variable.

To isolate cores specified in `isolcpus`:

```console
irqbalance --foreground --oneshot
```

To isolate core 3 (hexadecimal bitmask 0x8):

```console
IRQBALANCE_BANNED_CPUS=8 irqbalance --foreground --oneshot
```

List CPU affinity for all IRQs:

```console
find /proc/irq/ -name smp_affinity_list -print -exec cat '{}' ';'
```

Finally verify that isolated cores are not receiving interrupts by monitoring */proc/interrupts*:

```console
watch cat /proc/interrupts
```

References:

- The Linux Kernel Documentation. “SMP IRQ affinity”. https://www.kernel.org/doc/html/latest/core-api/irq/irq-affinity.html
- irqbalance. https://github.com/Irqbalance/irqbalance, https://linux.die.net/man/1/irqbalance

### Network stack tuning

For low latency networking I don’t recommend using the Linux kernel networking stack. Instead I recommend using kernel bypass technologies such as [DPDK](https://www.dpdk.org/), [OpenOnload](https://www.openonload.org/), [Mellanox VMA](https://www.mellanox.com/products/software/accelerator-software/vma?mtag=vma) or [Exablaze](https://exablaze.com/). If you are using the kernel networking stack there are several options you can use to tune it for low latency. I suggest reading the [Red Hat Enterprise Linux Performance Tuning Guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/performance_tuning_guide/sect-red_hat_enterprise_linux-performance_tuning_guide-networking-configuration_tools) and the blog post [“How to achieve low latency with 10Gbps Ethernet”](https://blog.cloudflare.com/how-to-achieve-low-latency/) to learn about these options.

### Disable swap

Accessing anonymous memory that has been swapped out to disk will incur a major page fault. You can prevent this by disabling swap:

```console
swapoff -a
```

Your application will still incur major page faults when accessing *file backed memory mappings* when the data is not in the page cache. Instead of disabling swap or in addition to disabling swap you can call [`mlockall(MCL_CURRENT|MCL_FUTURE)`](https://man7.org/linux/man-pages/man2/mlockall.2.html) to prevent all future page faults.

### Disable transparent huge pages

[Linux *transparent huge page* (THP) support](https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html) allows the kernel to automatically promote regular memory pages into huge pages. Huge pages reduces TLB pressure, but THP support introduces latency spikes when pages are promoted into huge pages and when memory compaction is triggered.

Disable transparent huge page support by supplying the kernel command line parameter `transparent_hugepage=never` or running the following command:

```console
echo never > /sys/kernel/mm/transparent_hugepage/enabled
```

References:

- “Transparent Hugepage Support”. https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html
- “Transparent Hugepages: measuring the performance impact”. https://alexandrnikitin.github.io/blog/transparent-hugepages-measuring-the-performance-impact/
- Erik Rigtorp. “Latency implications of virtual memory”. https://rigtorp.se/virtual-memory/
- Jonathan Corbet. “Memory compaction”. https://lwn.net/Articles/368869/
- Nitin Gupta. “Proactive compaction for the kernel”. https://lwn.net/Articles/817905/
- Nitin Gupta. “Proactive Compaction for the Linux kernel”. https://nitingupta.dev/post/proactive-compaction/

### Disable automatic NUMA memory balancing

Linux supports [automatic page fault based NUMA memory balancing](https://www.kernel.org/doc/html/latest/admin-guide/sysctl/kernel.html#numa-balancing) and [manual page migration](https://www.kernel.org/doc/html/latest/vm/page_migration.html) of memory between NUMA nodes. Migration of memory pages between NUMA nodes will cause TLB shootdowns and page faults for applications using the affected memory.

Automatic NUMA memory balancing can be disabled with the following command:

```console
echo 0 > /proc/sys/kernel/numa_balancing
```

Also make sure to disable the [*numad*](https://linux.die.net/man/8/numad) user space NUMA memory balancing service.

### Disable kernel samepage merging

[Linux kernel samepage merging (KSM)](https://www.kernel.org/doc/html/latest/admin-guide/mm/ksm.html) is a feature that de-duplicates memory pages that contains identical data. The merging process needs to lock the page tables and issue TLB shootdowns, leading to unpredictable memory access latencies. KSM only operates on memory pages that has been opted in to samepage merging using `madvise(...MADV_MERGEABLE)`. If needed KSM can be disabled system wide by running the following command:

```console
echo 0 > /sys/kernel/mm/ksm/run
```

### Disable mitigations for CPU vulnerabilities

This is application dependent, but consider disabling mitigations for CPU vulnerabilities. The [mitigations can have considerable performance impact](https://www.phoronix.com/scan.php?page=article&item=intel-10900k-mitigations&num=1) on system performance. Add [`mitigations=off`](https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html?highlight=mitigations) to your kernel command line to disable all mitigations.

Also consider using older CPU microcode without the microcode mitigations for CPU vulnerabilities.

### Use cache partitioning

If your processor supports cache partitioning (Intel Cache Allocation Technology) consider using it to allocate most of the last-level cache (LLC) to your application.

References:

- https://github.com/intel/intel-cmt-cat
- https://danluu.com/intel-cat/
- https://software.intel.com/en-us/articles/introduction-to-cache-allocation-technology

## Application design and tuning

### Prevent page faults

Use the [`mlockall(MCL_CURRENT|MCL_FUTURE)`](https://man7.org/linux/man-pages/man2/mlockall.2.html) system call to prevent page faults due to page cache eviction or swapping.

### Consider NUMA topology

TODO

### Use huge pages

The *translation lookaside buffer* (TLB) has a limited number of entries. If your application tries to access a memory page that is missing in the TLB, it causes a *TLB miss* requiring the MMU to walk the page table. The default page size is 4096 bytes, by using huge pages of 2 MB or 1 GB you can reduce the amount of TLB misses for the same amount of actively used RAM.

You can monitor TLB misses with the `perf` tool:

```console
# perf stat -e 'dTLB-loads,dTLB-load-misses,iTLB-loads,iTLB-load-misses' -a --timeout 10000

 Performance counter stats for 'system wide':

        10,525,620      dTLB-loads
         2,964,792      dTLB-load-misses          #   28.17% of all dTLB cache hits
         1,998,243      iTLB-loads
         1,068,635      iTLB-load-misses          #   53.48% of all iTLB cache hits

      10.002451754 seconds time elapsed
```

The above output shows the fraction of data loads (dTLB) and instruction loads (iTLB) that miss. If you observe a large fraction of TLB misses, you should consider using huge pages to reduce the number of TLB misses. There are additional CPU performance counters you can use to measure TLB pressure, consult your processor manual for a complete list of TLB related performance counters.

References:

- Wikipedia. “Translation lookaside buffer”. https://en.wikipedia.org/wiki/Translation_lookaside_buffer
- Ulrich Drepper. “Memory part 3: Virtual Memory”. https://lwn.net/Articles/253361/
- “Transparent Hugepages: measuring the performance impact”. https://alexandrnikitin.github.io/blog/transparent-hugepages-measuring-the-performance-impact/

### TLB shootdowns

Each process has a *page table* [mapping virtual address to physical address](https://lwn.net/Articles/253361/). When the page table changes such that memory is unmapped (`munmap`) or access to memory is restricted (`mmap` changing `PROT_*` flags) the TLB needs to be flushed on all cores currently running the application process. This is called a *TLB shootdown* and is implemented as a inter-processor interrupt (IPI) that will introduce jitter to your running application. In addition the subsequent TLB misses introduces memory access latency. Other causes of TLB shootdowns are: [transparent huge pages (THP)](https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html), [memory compaction](https://lwn.net/Articles/368869/), [kernel samepage merging (KSM)](https://www.kernel.org/doc/html/latest/admin-guide/mm/ksm.html), [page migration](https://www.kernel.org/doc/html/latest/vm/page_migration.html) and page cache writeback.

To avoid TLB shootdowns:

- Never release memory back to the kernel (`madvise(...MADV_FREE)`/`munmap`)
- Disable [transparent huge pages (THP)](https://www.kernel.org/doc/html/latest/admin-guide/mm/transhuge.html)
- Disable [NUMA balancing / page migration](https://www.kernel.org/doc/html/latest/vm/page_migration.html)
- Don’t create any file backed (or really page cache backed) writable memory mappings (`mmap(...PROT_WRITE`). Memory mappings of files on *tmpfs* and *hugetlbfs* is fine.

You can view the number of TLB shootdowns per CPU core in `/proc/interrupts`:

```console
$ egrep 'TLB|CPU' /proc/interrupts
            CPU0       CPU1       CPU2       CPU3
 TLB:   16642971   16737647   16870842   16350398   TLB shootdowns
```

You can monitor the number of TLB flushes system wide or per process using *perf*:

```console
# perf stat -e 'tlb:tlb_flush' -a -A --timeout 10000
```

References:

- Stackverflow. “Who performs the TLB shootdown?”. https://stackoverflow.com/questions/50256740/who-performs-the-tlb-shootdown
- Stackoverflow. “What is TLB shootdown?”. https://stackoverflow.com/questions/3748384/what-is-tlb-shootdown
- Wikipedia. “Inter-processor interrupt”. https://en.wikipedia.org/wiki/Inter-processor_interrupt

### Scheduling policy and real-time throttling

For lowest latency applications I avoid using real-time priorities `SCHED_FIFO` / `SCHED_RR`. Instead it’s better to run a single thread in `SCHED_OTHER` per core and using busy waiting / polling in order to never enter kernel mode. If you do so with real-time priority you can prevent the kernel from running tasks such as vmstat leading to kernel lockup issues.

To prevent accidental lockups the kernel comes with a feature that by default throttles real-time tasks to use at most 95% of the CPU bandwidth. If you are using real-time tasks you might want to adjust the real-time throttling configuration.

References:

- “Real Time Throttling”. https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/real_time_throttling
- “Real-Time group scheduling”. https://www.kernel.org/doc/html/latest/scheduler/sched-rt-group.html