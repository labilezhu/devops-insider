---
tags:
- cloud
- kernel
- performance
title: CPU 负载测量误差
---

CPU 负载测量误差

# CPU load[](https://www.kernel.org/doc/html/latest/admin-guide/cpu-load.html#cpu-load "Permalink to this headline")

Linux exports various bits of information via `/proc/stat` and `/proc/uptime` that userland tools, such as top(1), use to calculate the average time system spent in a particular state, for example:

```
 $ iostat
Linux 2.6.18.3-exp (linmac)     02/20/2007

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
          10.01    0.00    2.92    5.44    0.00   81.63
       
...
```


Here the system thinks that over the default sampling period the system spent 10.01% of the time doing work in user space, 2.92% in the kernel, and was overall 81.63% of the time idle.

In most cases the `/proc/stat` information reflects the reality quite closely, however due to the nature of how/when the kernel collects this data sometimes it can not be trusted at all.

So how is this information collected? Whenever timer interrupt is signalled the kernel looks what kind of task was running at this moment and increments the counter that corresponds to this tasks kind/state. The problem with this is that the system could have switched between various states multiple times between two timer interrupts yet the counter is incremented only for the last state.

## Example[](https://www.kernel.org/doc/html/latest/admin-guide/cpu-load.html#example "Permalink to this headline")

If we imagine the system with one task that periodically burns cycles in the following manner:

```
     time line between two timer interrupts
    |--------------------------------------|
     ^                                    ^
     |_ something begins working          |
                                          |_ something goes to sleep
                                         (only to be awaken quite soon)
```

In the above situation the system will be 0% loaded according to the `/proc/stat` (since the timer interrupt will always happen when the system is executing the idle handler), but in reality the load is closer to 99%.





## Ref
https://www.kernel.org/doc/html/latest/admin-guide/cpu-load.html