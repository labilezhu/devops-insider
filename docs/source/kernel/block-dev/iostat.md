# iostat

## Some problems with iostat on Linux

> From: https://utcc.utoronto.ca/~cks/space/blog/linux/IostatProblems March 14, 2006



 was recently reminded that Linux's `iostat` command is what I call 'overly helpful'. It's not that it lies to you, exactly; it's that `iostat` is a little bit too eager to please people reading its output, particularly `iostat -x` output. There are a number of issues.

(The rest of this assumes that you're familiar with the `iostat -x` fields.)

The fatally flawed field is `svctm`, the 'average service time' for IOs. It would be really nice to have this number, but unfortunately the kernel does not provide it; instead, `iostat` makes it up from other numbers using inaccurate assumptions, including that your disk only handles one request at a time.

The kernel accumulates statistics on a running basis; `iostat` derives per-second numbers by taking snapshots and computing the delta between them. Sometimes (usually under high load) some of the kernel statistics will effectively run backwards, with the new reading having smaller values than the old one. When this happens, `iostat` doesn't really notice. If you're lucky, the displayed stats are obviously wrong.

The rkB/s and wkB/s fields are redundant; they are literally just rsec/s and wsec/s divided by two. You might ask 'what if the device doesn't have 512 byte sectors?', and the answer is it doesn't matter; the general kernel IO system assumes 512-byte sectors, and in fact the kernel only reports sector information.

Iostat accurately documents `%iowait` as:

> Show the percentage of time that the CPU or CPUs were idle during which the system had an outstanding disk I/O request.

However, note that this is *not* the same thing as 'the percentage of time one or more processes were waiting on IO', since there are a number of background kernel activities that can queue IO while processes are idle waiting for unrelated things.

**Update, April 25th**: it turns out that the iostat manpage is wrong about what `%iowait` measures. See [LinuxIowait](https://utcc.utoronto.ca/~cks/space/blog/linux/LinuxIowait) for details.

PS: In Debian, Fedora Core, and I believe Red Hat Enterprise the `iostat` command, manpage, etc is part of the `sysstat` package (RPM, .deb, etc).

## What disk IO stats you get from the Linux kernel

> https://utcc.utoronto.ca/~cks/space/blog/linux/DiskIOStats  March 30, 2006

To follow up my previous entry on [iostat problems](https://utcc.utoronto.ca/~cks/space/blog/linux/IostatProblems), here's a rundown of the information you actually get from the Linux kernel.

First off, you only get this from 2.6 kernels, or 2.4 kernels with the Red Hat disk stats patch (such as Red Hat Enterprise 3). In 2.6 this information appears in `/proc/diskstats`; in Red Hat's 2.4, it appears in `/proc/partitions` with slightly more fields.

`/proc/diskstats` fields for *devices* (as opposed to partitions) are, in order (and using the names Red Hat labeled them with):

> major minor name rio rmerge rsect ruse wio wmerge wsect wuse running use aveq

In `/proc/diskstats`, partitions show only the major, minor, name, rio, rsect, wio, and wsect fields. In the Red Hat 2.4 code, `/proc/partitions` shows all fields for partitions, although you're still probably better off using the device.

These mean:

| rio         | number of read IO requests completed                         |
| ----------- | ------------------------------------------------------------ |
| rmerge      | number of submitted read requests that were merged into existing requests. |
| rsect       | number of read IO sectors submitted                          |
| ruse        | total length of time all completed read requests have taken to date, in milliseconds |
| w* versions | same as the r* versions, but for writes.                     |
| running     | instantaneous count of IOs currently in flight               |
| use         | how many milliseconds there has been at least one IO in flight |
| aveq        | the sum of how long all requests have spent in flight, in milliseconds |

Just to confuse everyone, the sector and merge counts are for *`submitted`* IO requests, but rio/wio and ruse/wuse are for *`completed`* IO requests. If IO is slow, bursty, or both, this difference can be important when trying to compute accurate numbers for things like the average sectors per request. (I've usually seen this for large writes during high IO load.)

The `aveq` number is almost but not quite the sum of `ruse` and `wuse`, because it also counts incomplete requests. All of `ruse`, `wuse`, `use`, and `aveq` can occasionally run backwards.

We can now see how `iostat` computes several fields:

| **`iostat` field** | **Computed as**               | **what**                              |
| ------------------ | ----------------------------- | ------------------------------------- |
| avgrq-sz           | (rsect + wsect) / (rio + wio) | the average sectors per request       |
| avgqu-sz           | (aveq / use)                  | the average queue size                |
| await              | (ruse + wuse) / (rio + wio)   | the average time to completion for IO |

While it would be useful to show 'rgrp-sz', 'wgrp-sz', 'rwait', and 'wwait' figures, `iostat` does not do so. This is unfortunate, as read and write IOs usually have very different characteristics (eg, typical write IO requests usually take significantly longer to complete than reads).

We can also see how the `iostat` `svtcm` field, the average IO service time, is bogus: there is simply *no information* on that provided by the kernel. The kernel would need a 'rduse' / 'wduse' set of fields that reported the total time taken once the requests had been picked up by the device driver (and it'd need to record the information).

(If you care, `iostat` computes `svtcm` as 'use / (rio + wio)'. This is less than obvious in the source code, because you have to cancel out a number of other terms. Also, it shows why `svctm` *drops* as your IO load rises (once you've hit 100% utilization).)

If you want to check the kernel code that does the work, it's in `drivers/block` in `ll_rw_blk.c` and `genhd.c`, in both 2.6 and Red Hat 2.4. `ll_rw_blk.c` maintains the numbers; `genhd.c` displays them.
