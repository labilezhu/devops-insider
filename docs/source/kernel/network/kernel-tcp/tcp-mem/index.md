---
title: 容器化 TCP Socket 缓存、接收窗口参数
date: 2022-10-02T23:22:15+08:00
draft: false
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
description: 最近需要支持一个单实例 TCP 连接数上 10k 的基础服务(Cassandra)的容器化。需要对其使用的资源（特别是TCP缓存内存）、对相邻 Pod（同一  worker node 上运行的）影响（即容器隔离情况），等时行预估。故写本文，以备忘
tags:
- TCP
- kernel
- socket
---

# TCP Socket 缓存、接收窗口参数


![image-20221014112014044](index.assets/logo.png)



最近需要支持一个单 POD 的 TCP 连接数上 10k 的基础服务(Cassandra)的容器化。需要对其使用的资源（特别是TCP缓存内存），以及对相邻 Pod（同一  worker node 上运行的）影响（即容器隔离情况），等进行预估。故写本文，以备忘。希望对读者也有一定参考价值，毕竟做技术要较真，要么有时间和能力就自己看内核源码，如果不能，要看文档和文章的话，只能货比三家才靠谱。

资料搜集不单单是个技术活，也是语言艺术活。如，输入什么关键字才合适、哪些资料来源看来更靠谱……

由于是资料收集，加上本人的翻译水平有限，所以我是尽量少翻译，保持原文，谢谢理解。同时我会加入一些个人注解，以供参考。



## 背景

最近需要支持一个单 POD 的 TCP 连接数上 10k 的基础服务(Cassandra)的容器化。需要对其使用的资源（特别是TCP缓存内存），以及对相邻 Pod（同一  worker node 上运行的）影响（即容器隔离情况），等进行预估。Cassandra 的官网推崇使用的 TCP 参数：

> https://docs.datastax.com/en/cassandra-oss/3.x/cassandra/install/installRecommendSettings.html#TCPsettings

```
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.core.rmem_default = 16777216
net.core.wmem_default = 16777216
net.core.optmem_max = 40960
net.ipv4.tcp_rmem = 4096 87380 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216
```

Cassandra 是容器化前的产品，官网的旧说明当然是假设这个 worknode / VM 上只跑 Cassandra 的情况下适用。它不会去帮我考虑在 k8s 环境，应用与 Cassandra 混合部署，甚至是使用 Ceph PVC 作为存储层的场景（虽然官网已经说了，[不建议用SAN](https://www.datastax.com/blog/impact-shared-storage-apache-cassandra), [cassandra.apache.org](https://www.mail-archive.com/user@cassandra.apache.org/msg40729.html#:~:text=I%20seriously%20doubt%20if%20C*%20could%20even%20work%20out%20of%20the%20box%20%0Awith%20yet%20another%20level%20of%20replication%20%26%20rebalancing)）。

那么问题来了。这个参数会对其它 POD带来什么影响？要回答这个问题，首先要知道：

- 这些参数是做什么的
- 背后的机理是怎样的
- 这些值比原来默认值增加/减少了多少

本文尝试为前两点作一些资料收集与总结。

## 说说历史

### TCP High Performance Extensions(RFC 1323)

> https://man7.org/linux/man-pages/man7/tcp.7.html#:~:text=sockets.%0A%0A%20%20%20%20%20%20%20Linux%20supports-,RFC%201323,-TCP%20high%20performance

Linux supports RFC 1323 TCP high performance extensions. These include `Protection Against Wrapped Sequence Numbers (PAWS)`, `Window Scaling` and `Timestamps`. **`Window scaling`** allows the use of large (> 64 kB) TCP windows in order to support links with high latency or bandwidth. To make use of them, the send and receive buffer sizes must be increased. They can be set :

- on individual sockets by using the **SO\_SNDBUF** and **SO\_RCVBUF** socket options with the [setsockopt(2)](https://man7.org/linux/man-pages/man2/setsockopt.2.html) call. 
- globally with the _`/proc/sys/net/ipv4/tcp_wmem`_ and _`/proc/sys/net/ipv4/tcp_rmem`_ files

The maximum sizes for socket buffers declared via the **SO\_SNDBUF** and **SO\_RCVBUF** mechanisms are limited by the values in the _`/proc/sys/net/core/rmem_max`_ and _`/proc/sys/net/core/wmem_max`_ files. Note that <mark>TCP actually allocates twice the size of the buffer requested in the [setsockopt(2)](https://man7.org/linux/man-pages/man2/setsockopt.2.html) call</mark>, and so a succeeding [getsockopt(2)](https://man7.org/linux/man-pages/man2/getsockopt.2.html) call will not return the same size of buffer as requested in the [setsockopt(2)](https://man7.org/linux/man-pages/man2/setsockopt.2.html) call. TCP uses the extra space for administrative purposes and internal kernel structures, and the _/proc_ file values reflect the larger sizes compared to the actual TCP windows. On individual connections, the socket buffer size must be set prior to the [listen(2)](https://man7.org/linux/man-pages/man2/listen.2.html) or [connect(2)](https://man7.org/linux/man-pages/man2/connect.2.html) calls in order to have it take effect. See [socket(7)](https://man7.org/linux/man-pages/man7/socket.7.html) for more information.

Kernel 提供两种调整 TCP 接收窗口大小的方法：

* 应用程序手工调整窗口与缓存大小

  调用  [setsockopt(2)](https://man7.org/linux/man-pages/man2/setsockopt.2.html) ，指定 `SO_SNDBUF` 与 `SO_RCVBUF` 。

* 内核自动调整

  使用 _`/proc/sys/net/ipv4/tcp_wmem`_ 与 _`/proc/sys/net/ipv4/tcp_rmem`_  两个参数。



### Kernel 自动调整窗口与缓存大小

> https://blog.cloudflare.com/optimizing-tcp-for-high-throughput-and-low-latency/#:~:text=is%20Linux%20autotuning.-,Linux%20autotuning,-Linux%20autotuning%20is

Linux autotuning is logic in the Linux kernel that adjusts the buffer size limits and the receive window based on actual packet processing. It takes into consideration a number of things including 

- TCP session RTT, 
- L7 read rates, 
- and the amount of available host memory.

Autotuning can sometimes seem mysterious, but it is actually fairly straightforward.

The central idea is that Linux can track the rate at which the local application is reading data off of the receive queue. It also knows the session RTT. Because Linux knows these things, it can automatically increase the buffers and receive window until it reaches the point at which the application layer or network bottleneck links are the constraint on throughput (and not host buffer settings). At the same time, autotuning prevents slow local readers from having excessively large receive queues. The way autotuning does that is by limiting the receive window and its corresponding receive buffer to an appropriate size for each socket.

The values set by autotuning can be seen via the Linux “`ss`” command from the iproute package (e.g. “`ss -tmi`”).  The relevant output fields from that command are:

> 💡 使用 `ss` 可查看 TCP socket 的缓存使用情况，可见我的 Blog: [可能是最完整的 TCP 连接健康指标工具 ss 的说明](https://blog.mygraphql.com/zh/notes/low-tec/network/tcp-inspect/#%E5%86%85%E5%AD%98tcp-windowtcp-buffer-%E7%9B%B8%E5%85%B3)
>
> [这里](https://blog.mygraphql.com/zh/notes/low-tec/network/tcp-inspect/#rcv_ssthresh) 同时提供一个例子。

`Recv-Q` is the number of user payload bytes not yet read by the local application.

`rcv_ssthresh` is the window clamp, a.k.a. the `maximum receive window size`. This value is not known to the sender. The sender receives only the `current window size`, via the TCP header field. A closely-related field in the kernel, `tp->window_clamp`, is the maximum window size allowable based on the amount of available memory. `rcv_ssthresh` is the receiver-side slow-start threshold value.

`skmem_r` is the actual amount of memory that is allocated, which includes not only user payload (`Recv-Q`) but also additional memory needed by Linux to process the packet (`packet metadata`). This is known within the kernel as `sk_rmem_alloc`.

*Note that there are other buffers associated with a socket, so `skmem_r` does not represent the total memory that a socket might have allocated.*

`skmem_rb` is the maximum amount of memory that could be allocated by the socket for the receive buffer. This is higher than `rcv_ssthresh` to account for memory needed for packet processing that is not packet data. Autotuning can increase this value (up to `tcp_rmem` max) based on how fast the L7 application is able to read data from the socket and the RTT of the session. This is known within the kernel as `sk_rcvbuf`.

`rcv_space` is the high water mark of the rate of the local application reading from the receive buffer during any RTT. This is used internally within the kernel to adjust `sk_rcvbuf`.

Earlier we mentioned a setting called `tcp_rmem`. `net.ipv4.tcp_rmem` consists of three values, but in this document we are always referring to the third value (except where noted). It is a global setting that specifies the maximum amount of memory that any TCP receive buffer can allocate, i.e. the maximum permissible value that autotuning can use for `sk_rcvbuf`. This is essentially just a failsafe for autotuning, and under normal circumstances should play only a minor role in TCP memory management.

It’s worth mentioning that receive buffer memory is not preallocated. <mark>Memory is allocated based on actual packets arriving and sitting in the receive queue.</mark> It’s also important to realize that filling up a receive queue is not one of the criteria that autotuning uses to increase `sk_rcvbuf`. Indeed, preventing this type of excessive buffering (bufferbloat) is one of the benefits of autotuning.



### 容器化/Namespaceify

自从容器和 linux namespace 成为主流后。Linux 一直在容器化这些参数的路上：

- https://github.com/torvalds/linux/search?q=Namespaceify&type=commits

从 Linux Kernel  v4.15 起，`net.ipv4.tcp_rmem` 与 `net.ipv4.tcp_wmem` 已经容器化了。即不同的容器/Linux Network Namespace 可以独立配置：

- [Kernel commit: tcp: Namespace-ify sysctl_tcp_rmem and sysctl_tcp_wmem](https://github.com/torvalds/linux/commit/356d1833b638bd465672aefeb71def3ab93fc17d)
- [How does non-namespace specific sysctl variables work in the context of network namespaces?](https://serverfault.com/questions/892399/how-does-non-namespace-specific-sysctl-variables-work-in-the-context-of-network)
- [slirp4netns](https://manpages.ubuntu.com/manpages/focal/man1/slirp4netns.1.html#:~:text=inside%20%20the%0A%20%20%20%20%20%20%20namespace)

每个 POD 独立配置这些参数的好处是，不同 POD 的不同应用类似，TCP 的使用可以千差万别：

- 有的要高吞吐和可以高延迟，缓存可以大点
- 有的要低延迟，准实时，但连接数量巨大，所以每个连接的缓存不宜太大

有的应用可以选择使用  [setsockopt(2)](https://man7.org/linux/man-pages/man2/setsockopt.2.html)   的 `SO_SNDBUF` and `SO_RCVBUF` 去配置。但不是所有应用都适合静态指定，或者都能配置。这时，容器化的配置就可以用上了。

## TCP 的 sysctl 配置



### rmem_default

> https://www.kernel.org/doc/html/latest/admin-guide/sysctl/net.html?highlight=optmem_max#rmem-default

The default setting of the socket receive buffer in bytes.

<mark>Deprecated for TCP socket. TCP 连接下只看 tcp_rmem。</mark>

### rmem_max

> https://www.kernel.org/doc/html/latest/admin-guide/sysctl/net.html?highlight=optmem_max#rmem-max

The maximum receive socket buffer size in bytes. Only for `setsockopt()`.

> https://cromwell-intl.com/open-source/performance-tuning/tcp.html
>
> maximum receive buffer sizes that can be set using `setsockopt()`, in bytes

### wmem_default

> https://www.kernel.org/doc/html/latest/admin-guide/sysctl/net.html?highlight=optmem_max#wmem-default

The default setting (in bytes) of the socket send buffer.

<mark>Deprecated for TCP socket. TCP 连接下只看 tcp_wmem。</mark>

### wmem_max

> https://www.kernel.org/doc/html/latest/admin-guide/sysctl/net.html?highlight=optmem_max#wmem-max

The maximum send socket buffer size in bytes.  Only for `setsockopt()`.

> https://cromwell-intl.com/open-source/performance-tuning/tcp.html
>
> maximum  send buffer sizes that can be set using `setsockopt()`, in bytes

### tcp_moderate_rcvbuf

> https://man7.org/linux/man-pages/man7/tcp.7.html

       tcp_moderate_rcvbuf (Boolean; default: enabled; since Linux
       2.4.17/2.6.7)
              If enabled, TCP performs receive buffer auto-tuning,
              attempting to automatically size the buffer (no greater
              than tcp_rmem[2]) to match the size required by the path
              for full throughput.

### net.ipv4.tcp_rmem

> [TCP.IP Illustrated Vol1]
>
> Large Buffers and Linux TCP Auto-Tuning

> https://www.ibm.com/docs/en/linux-on-systems?topic=tuning-tcpip-ipv4-settings

Contains three values that represent the `minimum`, `default` and `maximum` size of the TCP socket receive buffer.

The `minimum` represents the smallest receive buffer size guaranteed, even under memory pressure. The minimum value defaults to 1 page or 4096 bytes.

The `default` value represents the initial size of a TCP sockets receive buffer. This value supersedes(取而代之) `net.core.rmem_default` used by other protocols. The default value for this setting is 87,380 bytes. It also sets the `tcp_adv_win_scale` and initializes the TCP window size to 65535 bytes.

The `maximum` represents the largest receive buffer size automatically selected for TCP sockets. This value does NOT override `net.core.rmem_max`. The default value for this setting is somewhere between 87380 bytes and 6M bytes based on the amount of memory in the system.

The recommendation is to use the maximum value of 16M bytes or higher (kernel level dependent) especially for 10 Gigabit adapters.

> https://www.kernel.org/doc/html/latest/networking/ip-sysctl.html?highlight=tcp_wmem#:~:text=Default%3A%200-,tcp_rmem,-%2D%20vector%20of%203

min: Minimal size of receive buffer used by TCP sockets. It is guaranteed to each TCP socket, even under moderate memory pressure.

Default: 4K

default: initial size of receive buffer used by TCP sockets. This value overrides net.core.rmem_default used by other protocols. Default: 131072 bytes. This value results in initial window of 65535.

max: maximal size of receive buffer allowed for automatically selected receiver buffers for TCP socket. This value does not override `net.core.rmem_max`. Calling `setsockopt()` with `SO_RCVBUF` <mark>disables automatic tuning of that socket’s receive buffer size</mark>, in which case this value is ignored. Default: between 131072 and 6MB, depending on RAM size.

#### tcp_rmem 容器化/Namespaceify

根据一些信息，从 Linux Kernel  v4.15 起，`net.ipv4.tcp_rmem` 与 `net.ipv4.tcp_wmem` 已经容器化了。即不同的容器/Linux Network Namespace 可以独立配置：

- [Kernel commit: tcp: Namespace-ify sysctl_tcp_rmem and sysctl_tcp_wmem](https://github.com/torvalds/linux/commit/356d1833b638bd465672aefeb71def3ab93fc17d)
- [How does non-namespace specific sysctl variables work in the context of network namespaces?](https://serverfault.com/questions/892399/how-does-non-namespace-specific-sysctl-variables-work-in-the-context-of-network)
- [slirp4netns](https://manpages.ubuntu.com/manpages/focal/man1/slirp4netns.1.html#:~:text=inside%20%20the%0A%20%20%20%20%20%20%20namespace)

### net.ipv4.tcp_wmem

> [TCP.IP Illustrated Vol1]
>
> Large Buffers and Linux TCP Auto-Tuning



> https://www.kernel.org/doc/html/latest/networking/ip-sysctl.html?highlight=tcp_wmem#:~:text=defined%20in%20RFC1323.-,tcp_wmem,-%2D%20vector%20of%203

- `min`: Amount of memory reserved for send buffers for TCP sockets. Each TCP socket has rights to use it due to fact of its birth.

Default: 4K

- `default`: initial size of send buffer used by TCP sockets. This value overrides `net.core.wmem_default` used by other protocols.

It is usually lower than `net.core.wmem_default.`

Default: 16K

- `max`: Maximal amount of memory allowed for automatically tuned send buffers for TCP sockets. This value does not override `net.core.wmem_max`. Calling `setsockopt()` with `SO_SNDBUF` disables automatic tuning of that socket’s send buffer size, in which case this value is ignored.

Default: between 64K and 4MB, depending on RAM size.

#### tcp_wmem 容器化/Namespaceify

根据一些信息，从 Linux Kernel  v4.15 起，`net.ipv4.tcp_rmem` 与 `net.ipv4.tcp_wmem` 已经容器化了。即不同的容器/Linux Network Namespace 可以独立配置：

- [Kernel commit: tcp: Namespace-ify sysctl_tcp_rmem and sysctl_tcp_wmem](https://github.com/torvalds/linux/commit/356d1833b638bd465672aefeb71def3ab93fc17d)
- [How does non-namespace specific sysctl variables work in the context of network namespaces?](https://serverfault.com/questions/892399/how-does-non-namespace-specific-sysctl-variables-work-in-the-context-of-network)
- [slirp4netns](https://manpages.ubuntu.com/manpages/focal/man1/slirp4netns.1.html#:~:text=inside%20%20the%0A%20%20%20%20%20%20%20namespace)

### net.ipv4.tcp_mem

这是整机(node)级别上，为TCP内存使用配置一个限制/阀值。这个限制/阀值的计账，当然会包括：tcp_rmem / tcp_wmem 等的运行期使用内存。

> https://cromwell-intl.com/open-source/performance-tuning/tcp.html
>
> Parameter `tcp_mem` is the amount of memory in 4096-byte pages totaled across all TCP applications. It contains three numbers: the minimum, pressure, and maximum. The pressure is the threshold at which TCP will start to reclaim buffer memory to move memory use down toward the minimum. You want to avoid hitting that threshold.



> https://hechao.li/2022/09/30/a-tcp-timeout-investigation/
>
> Kernel error message when OOM:
>
> ```
> kernel: TCP: out of memory -- consider tuning tcp_mem
> ```
>
> Who sets tcp_mem in the first place?
> One observation is that the tcp_mem value is different on different instance types. An instance with a larger memory also has a larger tcp_mem value. Digging deeper, I found that by default this value is set by the Linux kernel using this formula. Also pasting the code here:
>
> ```c
> static void __init tcp_init_mem(void)
> {
> 	unsigned long limit = nr_free_buffer_pages() / 16;
> 
> 	limit = max(limit, 128UL);
> 	sysctl_tcp_mem[0] = limit / 4 * 3;		/* 4.68 % */
> 	sysctl_tcp_mem[1] = limit;			/* 6.25 % */
> 	sysctl_tcp_mem[2] = sysctl_tcp_mem[0] * 2;	/* 9.37 % */
> }
> ```





> https://man7.org/linux/man-pages/man7/tcp.7.html#:~:text=for%20full%20throughput.-,tcp_mem,-(since%20Linux%202.4

```
       tcp_mem (since Linux 2.4)
              This is a vector of 3 integers: [low, pressure, high].
              These bounds, measured in units of the system page size,
              are used by TCP to track its memory usage.  The defaults
              are calculated at boot time from the amount of available
              memory.  (TCP can only use low memory for this, which is
              limited to around 900 megabytes on 32-bit systems.  64-bit
              systems do not suffer this limitation.)

              low    TCP doesn't regulate its memory allocation when the
                     number of pages it has allocated globally is below
                     this number.

              pressure
                     When the amount of memory allocated by TCP exceeds
                     this number of pages, TCP moderates its memory
                     consumption.  This memory pressure state is exited
                     once the number of pages allocated falls below the
                     low mark.

              high   The maximum number of pages, globally, that TCP
                     will allocate.  This value overrides any other
                     limits imposed by the kernel.
```





### tcp_adv_win_scale

 (integer; default: 2; since Linux 2.4)

> https://man7.org/linux/man-pages/man7/tcp.7.html

Count buffering overhead as _bytes/2^tcp\_adv\_win\_scale_, if _tcp\_adv\_win\_scale_ is greater than 0; or _bytes-bytes/2^(-tcp\_adv\_win\_scale)_, if _tcp\_adv\_win\_scale_ is less than or equal to zero. 

The socket receive buffer space is shared between the application and kernel. 
- TCP maintains part of the buffer as the TCP window, this is the size of the receive window advertised to the other end. 
- The rest of the space is used as the "application" buffer, used to isolate the network from scheduling and application latencies. 

The _tcp\_adv\_win\_scale_ default value of 2 implies that the space used for the application buffer is one fourth that of the total.

> https://blog.cloudflare.com/optimizing-tcp-for-high-throughput-and-low-latency/#:~:text=high%2Dlatency%20networks.-,net.ipv4.tcp_adv_win_scale,-is%20a%20(non

`net.ipv4.tcp_adv_win_scale` is a (non-intuitive) number used to account for the overhead needed by Linux to process packets. The receive window is specified in terms of user payload bytes. Linux needs additional memory beyond that to track other data associated with packets it is processing.

The value of the receive window changes during the lifetime of a TCP session, depending on a number of factors. The maximum value that the receive window can be is limited by the amount of free memory available in the receive buffer, according to this table:

| tcp_adv_win_scale | TCP window size                             |
| ----------------- | ------------------------------------------- |
| 4                 | 15/16 * available memory in receive bufferf |
| 3                 | ⅞ * available memory in receive buffer      |
| 2                 | ¾ * available memory in receive buffer      |
| 1                 | ½ * available memory in receive buffer      |
| 0                 | available memory in receive buffer          |
| -1                | ½ * available memory in receive buffer      |
| -2                | ¼ * available memory in receive buffer      |
| -3                | ⅛ * available memory in receive buffer      |

We can intuitively (and correctly) understand that the amount of available memory in the receive buffer is the difference between the used memory and the maximum limit. But what is the maximum size a receive buffer can be? The answer is `sk_rcvbuf`.





### tcp_keepalive

| conf                          | Description                                                  |
| ----------------------------- | ------------------------------------------------------------ |
| net.ipv4.tcp_keepalive_intvl  | interval in seconds between subsequent keepalive probes.     |
| net.ipv4.tcp_keepalive_time   | interval in seconds before the first keepalive probe.        |
| net.ipv4.tcp_keepalive_probes | Maximum number of unacknowledged probes before the connection is considered dead. |

### net.core.somaxconn

> https://docs.kernel.org/networking/ip-sysctl.html?highlight=net+core+somaxconn#:~:text=%C2%B6-,somaxconn,-%2D%20INTEGER

Limit of socket `listen()` backlog, known in userspace as **`SOMAXCONN`**. Defaults to `4096`. (Was `128` before linux-5.4) See also `tcp_max_syn_backlog` for additional tuning for TCP sockets.

> 深入分析 somaxconn 容器化：http://arthurchiao.art/blog/the-mysterious-container-somaxconn/



### tcp_max_syn_backlog

> https://docs.kernel.org/networking/ip-sysctl.html?highlight=net+core+somaxconn#:~:text=of%20unswappable%20memory.-,tcp_max_syn_backlog,-%2D%20INTEGE

Maximal number of remembered connection requests (`SYN_RECV`), which have not received an `acknowledgment` from connecting client.

This is a per-listener limit.

The minimal value is 128 for low memory machines, and it will increase in proportion to the memory of machine.

If server suffers from overload, try increasing this number.

Remember to also check `/proc/sys/net/core/somaxconn` A `SYN_RECV` request socket consumes about 304 bytes of memory.



### SO_RCVBUF

> https://blog.cloudflare.com/optimizing-tcp-for-high-throughput-and-low-latency/#:~:text=answer%20is%20sk_rcvbuf.-,sk_rcvbuf,-is%20a%20per

`sk_rcvbuf` is a per-socket field that specifies the maximum amount of memory that a receive buffer can allocate. This can be set programmatically with the socket option `SO_RCVBUF`. This can sometimes be useful to do, for localhost TCP sessions, for example, but in general the use of `SO_RCVBUF` is not recommended.

So how is `sk_rcvbuf` set? The most appropriate value for that depends on the latency of the TCP session and other factors. This makes it difficult for L7 applications to know how to set these values correctly, as they will be different for every TCP session. The solution to this problem is Linux autotuning.





## CGroup Memory Control

容器化使用的资源隔离层 CGroup ，是可以用于限制容器的 TCP 相关内核内存使用的。但，kubernetes 好像暂时未支持作这个限制。

### Kernel CGroup Memory Control

内核 CGroup Memory TCP 控制说明：

> https://www.kernel.org/doc/Documentation/cgroup-v1/memory.txt
>
> Brief summary of control files.
>
> ```
>  memory.kmem.limit_in_bytes      # set/show hard limit for kernel memory
>  memory.kmem.usage_in_bytes      # show current kernel memory allocation
>  memory.kmem.failcnt             # show the number of kernel memory usage hits limits
>  memory.kmem.max_usage_in_bytes  # show max kernel memory usage recorded
>  
>  memory.kmem.tcp.limit_in_bytes  # set/show hard limit for tcp buf memory
>  memory.kmem.tcp.usage_in_bytes  # show current tcp buf memory allocation
>  memory.kmem.tcp.failcnt            # show the number of tcp buf memory usage hits limits
>  memory.kmem.tcp.max_usage_in_bytes # show max tcp buf memory usage recorded
> ```
>
> 2.7 Kernel Memory Extension (CONFIG_MEMCG_KMEM)
>
> With the Kernel memory extension, the Memory Controller is able to limit
> the amount of kernel memory used by the system. Kernel memory is fundamentally
> different than user memory, since it can't be swapped out, which makes it
> possible to DoS the system by consuming too much of this precious resource.
>
> Kernel memory accounting is enabled for all memory cgroups by default. But
> it can be disabled system-wide by passing `cgroup.memory=nokmem` to the kernel
> at boot time. In this case, kernel memory will not be accounted at all.
>
> Kernel memory limits are not imposed for the root cgroup. Usage for the root
> cgroup may or may not be accounted. The memory used is accumulated into
> `memory.kmem.usage_in_bytes`, or in a separate counter when it makes sense.
> (currently only for tcp).
> <mark>The main "kmem" counter is fed into the `main counter`, so kmem charges will
> also be visible from the user counter.</mark>
>
> Currently no soft limit is implemented for kernel memory. It is future work
> to trigger slab reclaim when those limits are reached.
>
> 2.7.1 Current Kernel Memory resources accounted
>
> * stack pages: every process consumes some stack pages. By accounting into
> kernel memory, we prevent new processes from being created when the kernel
> memory usage is too high.
>
> * slab pages: pages allocated by the SLAB or SLUB allocator are tracked. A copy
> of each `kmem_cache` is created every time the cache is touched by the first time
> from inside the `memcg`. The creation is done lazily, so some objects can still be
> skipped while the cache is being created. All objects in a slab page should
> belong to the same memcg. This only fails to hold when a task is migrated to a
> different memcg during the page allocation by the cache.
>
> * sockets memory pressure: some sockets protocols have memory pressure
> thresholds. The Memory Controller allows them to be controlled individually
> per cgroup, instead of globally.
>
> * <mark>tcp memory pressure</mark>: sockets memory pressure for the tcp protocol.
>
> 2.7.2 Common use cases
>
> Because the `kmem counter` is fed to the `main user counter`, <mark>kernel memory can
> never be limited completely independently of user memory</mark>. Say "U" is the user
> limit, and "K" the kernel limit. There are three possible ways limits can be
> set:
>
>     U != 0, K = unlimited:
>     This is the standard memcg limitation mechanism already present before kmem
>     accounting. Kernel memory is completely ignored.
>                 
>     U != 0, K < U:
>     Kernel memory is a subset of the user memory. This setup is useful in
>     deployments where the total amount of memory per-cgroup is overcommited.
>     Overcommiting kernel memory limits is definitely not recommended, since the
>     box can still run out of non-reclaimable memory.
>     In this case, the admin could set up K so that the sum of all groups is
>     never greater than the total memory, and freely set U at the cost of his
>     QoS.
>     WARNING: In the current implementation, memory reclaim will NOT be
>     triggered for a cgroup when it hits K while staying below U, which makes
>     this setup impractical.
>                 
>     U != 0, K >= U:
>     Since kmem charges will also be fed to the user counter and reclaim will be
>     triggered for the cgroup for both kinds of memory. This setup gives the
>     admin a unified view of memory, and it is also useful for people who just
>     want to track kernel memory usage.





> [lwn.net : per-cgroup tcp buffer pressure settings](https://lwn.net/Articles/459063/)
>
> This patch introduces per-cgroup tcp buffers limitation. This allows
> sysadmins to specify a maximum amount of kernel memory that
> tcp connections can use at any point in time. TCP is the main interest
> in this work, but extending it to other protocols would be easy.
>
> For this to work, I am hooking it into `memcg`, after the introdution of
> an extension for tracking and controlling objects in kernel memory.
> Since they are usually not found in page granularity, and are fundamentally
> different from userspace memory (not swappable, can't overcommit), they
> need their special place inside the Memory Controller.
>
> Right now, the `kmem` extension is quite basic, and just lays down the
> basic infrastucture for the ongoing work. 
>
> Although it does not account kernel memory allocated - I preferred to
> keep this series simple and leave accounting to the slab allocations when
> they arrive.
>
> What it does is to piggyback in(搭载) the memory control mechanism already present in
> `/proc/sys/net/ipv4/tcp_mem`. There is a `soft limit`, and a `hard limit`,
> that will suppress allocation when reached. For each cgroup, however,
> the file `kmem.tcp_maxmem` will be used to cap(限制) those values. 
>
> The usage I have in mind here is containers. Each container will
> define its own values for soft and hard limits, but none of them will
> be possibly bigger than the value the box' sysadmin specified from
> the outside.



> lwn.net: [Per-cgroup TCP buffer limits](https://lwn.net/Articles/470656/)

### Kubernetes

Kubernetes 好像暂时未支持作这个限制：

[Shall we add resource kernel-memory limit ？ #45476](https://github.com/kubernetes/kubernetes/issues/45476)

### Google Cloud: TCP Memory Isolation

以下是 Google Cloud 的一个关于 多租户环境下，内核 TCP 内存隔离的资料：

> [Google Cloud: TCP Memory Isolation on Multi-tenant Servers - Sep 13, 2022](https://lpc.events/event/16/contributions/1212/attachments/1079/2052/LPC%202022%20-%20TCP%20memory%20isolation.pdf)

#### How is TCP memory accounted?

* Accounting TCP memory 
  * ○ Single global counter: tcp_memory_allocated 
  * ○ Visible through `/proc/net/sockstat[6]` and `/proc/net/protocols`

* Limiting TCP memory 
  * ○ System wide shared limit: `/proc/sys/net/ipv4/tcp_mem` (Array of 3 long integers) 
    * ■ Enter TCP pressure state: tcp_memory_allocated > tcp_mem[1] 
    * ■ Leave TCP pressure state: tcp_memory_allocated <= tcp_mem[0] 
    * ■ Hard TCP usage limit: tcp_memory_allocated > tcp_mem[2]

#### What happens on TCP pressure?

Reduce (or prevent increasing) the send or receive buffers for the sockets ● 

- On RX 
  - ○ May coalesce packets 
  - ○ May drop packets preferably out-of-order packets 
  - ○ Wakes up the userspace application to consume the incoming packets ● 

- On TX ○ May throttle the current thread of the sender



#### Current TCP memory accounting causes isolation issues

Problem 1: Shared unregulated `tcp_mem` limit

When the TCP memory usage hit the TCP limit: 

1. Sockets of arbitrary jobs will see reduced send and receive buffer. 
2. Packets of arbitrary jobs will be drops. 
3. Threads of arbitrary jobs will get throttled. 

**Low priority jobs can hog TCP memory and adversely impact higher priority jobs**



#### Solving Problem 1

- Remove shared global TCP limit
- Start charging jobs for their TCP memory usage 
  - ○ Use memory cgroups to start charging TCP memory 
  - ○ The memcg limit of jobs will limit their TCP memory usage



#### Solution: TCP memory accounting using memory cgroups (TCP-memcg)

- ● TCP memory accounting has different semantics in memcg-v1 vs memcg-v2 
- ● In `memcg-v1`, TCP memory is accounted separately from the memcg memory usage 
  - ○ Added complexity to provision another resource 
  - ○ Off by default and inefficient 
- ● In `memcg-v2`, TCP memory is accounted as regular memory 
  - ○ Aligns with our cgroup v2 migration journey 



## TCP 参数容器化/Namespaceify

### Kernel

- [Kernel commit: tcp: Namespace-ify sysctl_tcp_rmem and sysctl_tcp_wmem](https://github.com/torvalds/linux/commit/356d1833b638bd465672aefeb71def3ab93fc17d)
- [Namespaceify more of the tcp sysctl knobs](https://lwn.net/Articles/674510/)
- https://github.com/torvalds/linux/search?q=Namespaceify&type=commits



### Kubernetes - TCP 参数容器化

> https://kubernetes.io/docs/tasks/administer-cluster/sysctl-cluster/

A number of sysctls are namespaced in today's Linux kernels. This means that they can be set independently for each pod on a node. Only namespaced sysctls are configurable via the pod `securityContext` within Kubernetes.

The following sysctls are known to be namespaced. This list could change in future versions of the Linux kernel.

- The parameters under `net.*` that can be set in container networking namespace. However, there are exceptions (e.g., `net.netfilter.nf_conntrack_max` and `net.netfilter.nf_conntrack_expect_max` can be set in container networking namespace but they are unnamespaced).

Sysctls with no namespace are called node-level sysctls. If you need to set them, you must manually configure them on each node's operating system, or by using a DaemonSet with privileged containers.

Use the pod `securityContext` to configure namespaced sysctls. The `securityContext` applies to all containers in the same pod.

This example uses the pod `securityContext` to set a safe `sysctl kernel.shm_rmid_forced` and two unsafe` sysctls net.core.somaxconn` and `kernel.msgmax`. There is no distinction between safe and unsafe sysctls in the specification.

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: sysctl-example
spec:
  securityContext:
    sysctls:
    - name: kernel.shm_rmid_forced
      value: "0"
    - name: net.core.somaxconn
      value: "1024"
    - name: kernel.msgmax
      value: "65536"
  ...
```



## 其它参考

> https://blog.cloudflare.com/optimizing-tcp-for-high-throughput-and-low-latency/
>
> https://medium.com/mercedes-benz-techinnovation-blog/tuning-network-sysctls-in-docker-and-kubernetes-766e05da4ff2