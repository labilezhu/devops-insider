# Delay accounting

> https://docs.kernel.org/accounting/delay-accounting.html



Tasks encounter delays in execution when they wait for some kernel resource to become available e.g. a runnable task may wait for a free CPU to run on.

The per-task delay accounting functionality measures the delays experienced by a task while

1. waiting for a CPU (while being runnable)
2. completion of synchronous block I/O initiated by the task
3. swapping in pages
4. memory reclaim
5. thrashing
6. direct compact
7. write-protect copy
8. IRQ/SOFTIRQ

and makes these statistics available to userspace through the taskstats interface.

Such delays provide feedback for setting a task's cpu priority, io priority and rss limit values appropriately. Long delays for important tasks could be a trigger for raising its corresponding priority.

The functionality, through its use of the taskstats interface, also provides delay statistics aggregated for all tasks (or threads) belonging to a thread group (corresponding to a traditional Unix process). This is a commonly needed aggregation that is more efficiently done by the kernel.

Userspace utilities, particularly resource management applications, can also aggregate delay statistics into arbitrary groups. To enable this, delay statistics of a task are available both during its lifetime as well as on its exit, ensuring continuous and complete monitoring can be done.

## Interface

Delay accounting uses the taskstats interface which is described in detail in a separate document in this directory. Taskstats returns a generic data structure to userspace corresponding to per-pid and per-tgid statistics. The delay accounting functionality populates specific fields of this structure. See for a description of the fields pertaining to delay accounting. It will generally be in the form of counters returning the cumulative delay seen for cpu, sync block I/O, swapin, memory reclaim, thrash page cache, direct compact, write-protect copy, IRQ/SOFTIRQ etc.

Taking the difference of two successive readings of a given counter (say cpu_delay_total) for a task will give the delay experienced by the task waiting for the corresponding resource in that interval.

When a task exits, records containing the per-task statistics are sent to userspace without requiring a command. If it is the last exiting task of a thread group, the per-tgid statistics are also sent. More details are given in the taskstats interface description.

The getdelays.c userspace utility in tools/accounting directory allows simple commands to be run and the corresponding delay statistics to be displayed. It also serves as an example of using the taskstats interface.

## Usage

Compile the kernel with:

```
CONFIG_TASK_DELAY_ACCT=y
CONFIG_TASKSTATS=y
```

Delay accounting is disabled by default at boot up. To enable, add:

```
delayacct
```

to the kernel boot options. The rest of the instructions below assume this has been done. Alternatively, use `sysctl kernel.task_delayacct` to switch the state at runtime. Note however that <mark>only tasks started after enabling it will have delayacct information</mark>.

```bash
sudo sysctl kernel.task_delayacct=1
```



After the system has booted up, use a utility similar to getdelays.c to access the delays seen by a given task or a task group (tgid). The utility also allows a given command to be executed and the corresponding delays to be seen.

General format of the getdelays command:

```
getdelays [-dilv] [-t tgid] [-p pid]
```

Get delays, since system boot, for pid 10:

```
# ./getdelays -d -p 10
(output similar to next case)
```

Get sum of delays, since system boot, for all pids with tgid 5:

```
# ./getdelays -d -t 5
print delayacct stats ON
TGID    5


CPU             count     real total  virtual total    delay total  delay average
                    8        7000000        6872122        3382277          0.423ms
IO              count    delay total  delay average
           0              0          0.000ms
SWAP            count    delay total  delay average
               0              0          0.000ms
RECLAIM         count    delay total  delay average
           0              0          0.000ms
THRASHING       count    delay total  delay average
               0              0          0.000ms
COMPACT         count    delay total  delay average
               0              0          0.000ms
WPCOPY          count    delay total  delay average
               0              0          0.000ms
IRQ             count    delay total  delay average
               0              0          0.000ms
```

Get IO accounting for pid 1, it works only with -p:

```
# ./getdelays -i -p 1
printing IO accounting
linuxrc: read=65536, write=0, cancelled_write=0
```



## task-struct



> https://elixir.bootlin.com/linux/v5.19.17/source/include/linux/sched.h#L1315
>
> ```c
> struct task_struct {
> #ifdef CONFIG_THREAD_INFO_IN_TASK
> 	/*
> 	 * For reasons of header soup (see current_thread_info()), this
> 	 * must be the first element of task_struct.
> 	 */
> 	struct thread_info		thread_info;
> #endif
> 	unsigned int			__state;
> ...
> 
> #ifdef CONFIG_TASK_DELAY_ACCT
> 	struct task_delay_info		*delays; // <<<<<---
> #endif
> ...
> ```
>
> https://elixir.bootlin.com/linux/v5.19.17/source/include/linux/delayacct.h#L13
>
> ```c
> struct task_delay_info {
> 	raw_spinlock_t	lock;
> 
> 	/* For each stat XXX, add following, aligned appropriately
> 	 *
> 	 * struct timespec XXX_start, XXX_end;
> 	 * u64 XXX_delay;
> 	 * u32 XXX_count;
> 	 *
> 	 * Atomicity of updates to XXX_delay, XXX_count protected by
> 	 * single lock above (split into XXX_lock if contention is an issue).
> 	 */
> 
> 	/*
> 	 * XXX_count is incremented on every XXX operation, the delay
> 	 * associated with the operation is added to XXX_delay.
> 	 * XXX_delay contains the accumulated delay time in nanoseconds.
> 	 */
> 	u64 blkio_start;
> 	u64 blkio_delay;	/* wait for sync block io completion */
> 	u64 swapin_start;
> 	u64 swapin_delay;	/* wait for swapin */
> 	u32 blkio_count;	/* total count of the number of sync block */
> 				/* io operations performed */
> 	u32 swapin_count;	/* total count of swapin */
> 
> 	u64 freepages_start;
> 	u64 freepages_delay;	/* wait for memory reclaim */
> 
> 	u64 thrashing_start;
> 	u64 thrashing_delay;	/* wait for thrashing page */
> 
> 	u64 compact_start;
> 	u64 compact_delay;	/* wait for memory compact */
> 
> 	u64 wpcopy_start;
> 	u64 wpcopy_delay;	/* wait for write-protect copy */
> 
> 	u32 freepages_count;	/* total count of memory reclaim */
> 	u32 thrashing_count;	/* total count of thrash waits */
> 	u32 compact_count;	/* total count of memory compact */
> 	u32 wpcopy_count;	/* total count of write-protect copy */
> };
> ```



## 记账

https://github.com/torvalds/linux/blob/3d7cb6b04c3f3115719235cc6866b10326de34cd/kernel/delayacct.c#L182

```c
int delayacct_add_tsk(struct taskstats *d, struct task_struct *tsk)
{
...
	d->blkio_count += tsk->delays->blkio_count;

```



https://github.com/torvalds/linux/blob/3d7cb6b04c3f3115719235cc6866b10326de34cd/kernel/taskstats.c#L186

```c
static void fill_stats(struct user_namespace *user_ns,
		       struct pid_namespace *pid_ns,
		       struct task_struct *tsk, struct taskstats *stats)
{
	delayacct_add_tsk(stats, tsk);
```





## /proc/pid/stat

https://elixir.bootlin.com/linux/v5.19.17/source/kernel/delayacct.c#L193

```c
__u64 __delayacct_blkio_ticks(struct task_struct *tsk)
{
	__u64 ret;
	unsigned long flags;

	raw_spin_lock_irqsave(&tsk->delays->lock, flags);
	ret = nsec_to_clock_t(tsk->delays->blkio_delay);
	raw_spin_unlock_irqrestore(&tsk->delays->lock, flags);
	return ret;
}
```



https://elixir.bootlin.com/linux/v5.19.17/source/fs/proc/array.c#L623

```c
static int do_task_stat(struct seq_file *m, struct pid_namespace *ns,
			struct pid *pid, struct task_struct *task, int whole)
{
...
	seq_put_decimal_ull(m, " ", delayacct_blkio_ticks(task));

```

> https://man7.org/linux/man-pages/man5/proc.5.html
>
> ```
>        /proc/pid/stat
>               Status information about the process.  This is used by
>               ps(1).  It is defined in the kernel source file
>               fs/proc/array.c.
> 		...              
>               (42) delayacct_blkio_ticks  %llu  (since Linux 2.6.18)
>                      Aggregated block I/O delays, measured in clock
>                      ticks (centiseconds).              
> ```



## pidstat

> https://manpages.ubuntu.com/manpages/focal/en/man1/pidstat.1.html
>
> ```
>        -d     Report I/O statistics (kernels 2.6.20 and later only).  The following values may be
>               displayed:
> ...
>               iodelay
>                      Block  I/O  delay of the task being monitored, measured in clock ticks. This
>                      metric includes the delays spent waiting for sync block I/O  completion  and
>                      for swapin block I/O completion.
> ```



> https://github.dev/sysstat/sysstat/blob/master/pidstat.c
>
> ```c
> /*
>  ***************************************************************************
>  * Read stats from /proc/#[/task/##]/stat.
>  *
>  * IN:
>  * @pid		Process whose stats are to be read.
>  * @plist	Pointer on the linked list where PID is saved.
>  * @tgid	If !=0, thread whose stats are to be read.
>  * @curr	Index in array for current sample statistics.
>  *
>  * OUT:
>  * @thread_nr	Number of threads of the process.
>  *
>  * RETURNS:
>  * 0 if stats have been successfully read, and 1 otherwise.
>  ***************************************************************************
>  */
> int read_proc_pid_stat(pid_t pid, struct st_pid *plist,
> 		       unsigned int *thread_nr, pid_t tgid, int curr)
> {
> 
> rc = sscanf(start,
> 		    "%*s %*d %*d %*d %*d %*d %*u %llu %llu"
> 		    " %llu %llu %llu %llu %lld %lld %*d %*d %u %*u %*d %llu %llu"
> 		    " %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u"
> 		    " %*u %u %u %u %llu %llu %lld\n",
> 		    &pst->minflt, &pst->cminflt, &pst->majflt, &pst->cmajflt,
> 		    &pst->utime,  &pst->stime, &pst->cutime, &pst->cstime,
> 		    thread_nr, &pst->vsz, &pst->rss, &pst->processor,
> 		    &pst->priority, &pst->policy,
> 		    &pst->blkio_swapin_delays, &pst->gtime, &pst->cgtime);
> ...
> }
> 
> 
> /*
>  ***************************************************************************
>  * Display I/O statistics.
>  *
>  * IN:
>  * @prev	Index in array where stats used as reference are.
>  * @curr	Index in array for current sample statistics.
>  * @dis		TRUE if a header line must be printed.
>  * @disp_avg	TRUE if average stats are displayed.
>  * @prev_string	String displayed at the beginning of a header line. This is
>  * 		the timestamp of the previous sample, or "Average" when
>  * 		displaying average stats.
>  * @curr_string	String displayed at the beginning of current sample stats.
>  * 		This is the timestamp of the current sample, or "Average"
>  * 		when displaying average stats.
>  * @itv		Interval of time in 1/100th of a second.
>  *
>  * RETURNS:
>  * 0 if all the processes to display have terminated.
>  * <> 0 if there are still some processes left to display.
>  ***************************************************************************
>  */
> int write_pid_io_stats(int prev, int curr, int dis, int disp_avg,
> 		       char *prev_string, char *curr_string,
> 		       unsigned long long itv)
> {
> 			cprintf_u64(NO_UNIT, 1, 7,
> 				    (unsigned long long) (pstc->blkio_swapin_delays - pstp->blkio_swapin_delays));    
> }
> 
> ```
>
> 



## sysctl kernel.task_delayacct

- [kernel commit: [tip: sched/core] delayacct: Add sysctl to enable at runtime. CommitterDate: Wed, 12 May 2021](https://www.spinics.net/lists/linux-tip-commits/msg57566.html)



`iotop` documentation:

> https://kaisenlinux.org/manpages/iotop.html
>
> iotop watches I/O usage information output by the Linux kernel (requires 2.6.20 or later) and displays a table of current I/O usage by processes or threads on the system. At least   the CONFIG_TASK_DELAY_ACCT, CONFIG_TASK_IO_ACCOUNTING, CONFIG_TASKSTATS and CONFIG_VM_EVENT_COUNTERS options need to be enabled in your Linux kernel build configuration and  since   Linux kernel 5.14, the `kernel.task_delayacct` sysctl enabled.
>
> 从 Linux 内核 5.14.x 开始，`kernel.task_delayacct` 可在运行时配置并默认设置为关闭。



## Test

```bash
sudo sysctl kernel.task_delayacct=1
```



```
root@labile-hp:/proc# echo $$
202484
root@labile-hp:/proc# exec dd if=/dev/nvme0n1p6 of=/dev/null bs=1k count=100000k
```



```
sudo pidstat -d -r -p 202484  -u 205:08:35 PM   UID       PID    %usr %system  %guest   %wait    %CPU   CPU  Command
05:08:37 PM     0    202484    9.50   66.00    0.00    0.00   75.50     1  dd

05:08:35 PM   UID       PID   kB_rd/s   kB_wr/s kB_ccwr/s iodelay  Command
05:08:37 PM     0    202484 1347648.00      0.00      0.00      49  dd

05:08:37 PM   UID       PID    %usr %system  %guest   %wait    %CPU   CPU  Command
05:08:39 PM     0    202484   10.00   82.00    0.00    0.00   92.00     1  dd

05:08:37 PM   UID       PID   kB_rd/s   kB_wr/s kB_ccwr/s iodelay  Command
05:08:39 PM     0    202484 1598144.00      0.00      0.00      15  dd
```





