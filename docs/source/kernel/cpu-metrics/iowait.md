# IOWait

## Linux's %iowait statistic



> From: https://utcc.utoronto.ca/~cks/space/blog/linux/LinuxIowait



The `iostat` manpage documents `%iowait` as:

> Show the percentage of time that the CPU or CPUs were idle during which the system had an outstanding disk I/O request.

It turns out that the manpage is wrong, which I found out by reading the kernel source because I was curious about what exactly it measured.

The actual definition of `%iowait` is <mark>**the percentage of the time that the system was idle and at least one process was waiting for disk IO to finish**</mark>. (This is true for both the 2.6 kernel and Red Hat's special 2.4 kernels with better disk IO statistics, including Red Hat Enterprise 3.)

(The actual kernel measure is the amount of time that each CPU has spent in each mode; it shows up in `/proc/stat`. `iostat` converts this to percentages.)

The difference may seem picky(过分讲究的), but it's important because not all IO causes processes to wait. For example, Linux doesn't immediately flush data written to files to disk; it does it later, in the background, when it's convenient. Under the manpage's definition, this background flushing of data would take a system from `%idle` into `%iowait`, as would slowly paging out unused bits of programs.

This means `%iowait` is roughly the amount of time that your system could have been doing useful work if the disks were faster. A climbing `%iowait` is a danger sign that your system may be running into an IO bottleneck. A low iowait is not necessarily an indication that you don't have an IO problem; you also want to look at things like the number of processes shown as blocked ('`b`' state) in `vmstat` output.

(Finding potential disk IO bottlenecks and troubleshooting them is a really big field, so this is in no way comprehensive advice.)



## Linux's iowait statistic and multi-CPU machines

> From: https://utcc.utoronto.ca/~cks/space/blog/linux/LinuxMultiCPUIowait  March 7, 2020

Yesterday I wrote about [how multi-CPU machines quietly complicate the standard definition of iowait](https://utcc.utoronto.ca/~cks/space/blog/unix/IowaitAndMultipleCPUs), because you can have some but not all CPUs idle while you have processes waiting on IO. The system is not totally idle, which is what [the normal Linux definition of iowait](https://utcc.utoronto.ca/~cks/space/blog/linux/LinuxIowait) is about, but some CPUs are idle and implicitly waiting for IO to finish. Linux complicates its life because iowait is considered to be a per-CPU statistic, like user, nice, system, idle, irq, softirq, and the other per-CPU times reported in `/proc/stat` (see [`proc(5)`](http://man7.org/linux/man-pages/man5/proc.5.html)).

> As it turns out, this per-CPU iowait figure is genuine, in one sense; it is computed separately for each CPU and CPUs may report significantly different numbers for it.
>
> 事实证明，从某种意义上来说，每个 CPU 的 iowait 数据是真实的；它是针对每个 CPU 单独计算的，并且 CPU 可能会报告显着不同的数字。



 How modern versions of the Linux kernel keep track of iowait involves something between brute force and hand-waving. 

Each *task* (a process or thread) is associated with a CPU while it is running. **When a task goes to sleep to wait for IO**, it increases a count of how many tasks are waiting for IO 'on' that CPU, called [nr_iowait](https://elixir.bootlin.com/linux/latest/source/kernel/sched/sched.h#L1030). **Then if `nr_iowait` is greater than zero and the CPU is `idle`**, the idle time is charged to `iowait` for that CPU instead of to '`idle`'.

> https://elixir.bootlin.com/linux/latest/source/kernel/sched/core.c#L5480

```c
/*
 * IO-wait accounting, and how it's mostly bollocks (on SMP).
 *
 * The idea behind IO-wait account is to account the idle time that we could
 * have spend running if it were not for IO. That is, if we were to improve the
 * storage performance, we'd have a proportional reduction in IO-wait time.
 *
 * This all works nicely on UP, where, when a task blocks on IO, we account
 * idle time as IO-wait, because if the storage were faster, it could've been
 * running and we'd not be idle.
 *
 * This has been extended to SMP, by doing the same for each CPU. This however
 * is broken.
 *
 * Imagine for instance the case where two tasks block on one CPU, only the one
 * CPU will have IO-wait accounted, while the other has regular idle. Even
 * though, if the storage were faster, both could've ran at the same time,
 * utilising both CPUs.
 *
 * This means, that when looking globally, the current IO-wait accounting on
 * SMP is a lower bound, by reason of under accounting.
 *
 * Worse, since the numbers are provided per CPU, they are sometimes
 * interpreted per CPU, and that is nonsensical. A blocked task isn't strictly
 * associated with any one particular CPU, it can wake to another CPU than it
 * blocked on. This means the per CPU IO-wait number is meaningless.
 *
 * Task CPU affinities can make all that even more 'interesting'.
 */

unsigned int nr_iowait(void)
{
	unsigned int i, sum = 0;

	for_each_possible_cpu(i)
		sum += nr_iowait_cpu(i);

	return sum;
}
```





(You can see this in the code in `account_idle_time()` in [kernel/sched/cputime.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/kernel/sched/cputime.c#n214).)

The problem with this is that a task waiting on IO is not really attached to any particular CPU. When it wakes up, the kernel will try to run it on its 'current' CPU (ie the last CPU it ran on, the CPU who's run queue it's in), but if that CPU is busy and another CPU is free, the now-awake task will be scheduled on that CPU. 



> 特殊情况：There is nothing that particularly guarantees that tasks waiting for IO are evenly distributed across all CPUs, or are parked on idle CPUs; as far as I know, you might have five tasks all waiting for IO on one CPU that's also busy running a sixth task, while five other CPUs are all idle. In this situation, the Linux kernel will happily say that one CPU is 100% user and five CPUs are 100% idle and there's no iowait going on at all.



(As far as I can see, the per-CPU number of tasks waiting for IO is not reported at all. A global number of tasks in iowait is reported as `procs_blocked` in `/proc/stat`, but that doesn't tell you how they're distributed across your CPUs. Also, it's an instantaneous number instead of some sort of accounting of this over time.)

There's a nice big comment about this in [kernel/sched/core.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/kernel/sched/core.c#n3444) (just above `nr_iowait()`, if you have to find it because the source has shifted). The comment summarizes the situation this way, emphasis mine:

> This means, that when looking globally, **the current IO-wait accounting on SMP is a lower bound**, by reason of under accounting.

(It also says in somewhat more words that looking at the iowait for individual CPUs is nonsensical.)

Programs that report per-CPU iowait numbers on Linux are in some sense not incorrect; they're faithfully reporting what the kernel is telling them. The information they present is misleading, though, and in an ideal world their documentation would tell you that per-CPU iowait is not meaningful and should be ignored unless you know what you're doing.

PS: It's possible that [`/proc/pressure/io`](https://www.kernel.org/doc/html/latest/accounting/psi.html) can provide useful information here, if you have a sufficiently modern kernel. Unfortunately the normal Ubuntu 18.04 server kernel is not sufficiently modern.
