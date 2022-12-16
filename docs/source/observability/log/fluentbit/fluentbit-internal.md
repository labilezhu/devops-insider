# 图解 Fluent Bit 内部设计

## 互动图片

> 📢  本文的正常打开方法是，点击 “*用 Draw.io 打开*” 后，进入互动图片状态。图中很多元素提供链接到相关源码或文档。可以做交叉参考，是进一步深入的入口，也是图可信性取证。
> 本文的大部分内容是放在图中了。看图比文字更重要。

## Record 概念

可以简化认为，日志文件中的一行，就是一个 `Record`。内部以 json 树形式来记录一个 `Record`。

为提高内存中的 `Record` 数据密度，同时加速 json 结构树的访问。Fluent Bit 内部使用了 [`MessagePack`](https://msgpack.org/index.html) 格式在内存与硬盘中保存数据。所以，请注意不是用我们日常见的明文 json 格式。可能如果要比较精细评估 Fluent Bit 内存使用时，需要考虑这一点。



## Chunk 概念

为提高处理性能，Fluent Bit 每次以小批量的 `Record`  为单位处理数据。每个批叫 `Chunk`。他是 `Record` 的集合。

数据在由 Input Plugin 加载入内存时，就已经是以批(`Chunk`) 的形式了。加载后，经由 pipeline、最后再到 Output，均以 Chunk 为粒度去处理（这里我未完全肯定）。



下面说明一下代码与存储的结构：



:::{figure-md} 图：Chunk 定义
:class: full-width

<img src="fluentbit-chunk.drawio.svg" alt="Chunk 定义">

*图：Chunk 定义*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-chunk.drawio.svg)*

## Pipeline/Engine 概念

:::{figure-md} 图：Engine 概念
:class: full-width

<img src="fluentbit-pipeline.drawio.svg" alt="Engine 概念">

*图：Engine 概念*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-pipeline.drawio.svg)*


## Input

### Tail Input

#### Tail Input 概述

:::{figure-md} 图：Tail Input 概述
:class: full-width

<img src="fluentbit-tail-input.drawio.svg" alt="Tail Input 概述">

*图：Tail Input 概述*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-tail-input.drawio.svg)*

#### Tail Input 内部设计

:::{figure-md} 图：Tail Input 内部设计
:class: full-width

<img src="fluentbit-tail-internal.drawio.svg" alt="Tail Input 内部设计">

*图：Tail Input 内部设计*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-tail-internal.drawio.svg)*



图中已经比较详细了，这里只想补充一些基本概念。

对于毎一个 Tail Input 实例，均有以下协程 (Collector)：

- watcher collector process
- static file collectior process
- pending file collector process

对于每一个 Tail Input 实例，还有以下协程:

- input path scan process



以下是一些推测的流程：



1. `input path scan process`  的主要职责是按 `Tail Input` 的 `path` 配置要求，定时([`Refresh_Interval`](https://docs.fluentbit.io/manual/pipeline/inputs/tail#:~:text=False-,Refresh_Interval,-The%20interval%20of))扫描，发现文件的：新增等情况。然后把发现通知到 `static file collectior`
2.  `static file collectior` 首先使用 inotify 去 watch 文件 。然后尝试一次读完文件，如果因各种原因无法一次完成（如内存不足），会通知到 `pending file collector` 去异步完成
3. `pending file collector` 完成文件的读取
4. Linux Kernel 在监测到文件有写入(`IN_MODIFY`)时，发马上读取文件。当发现文件被删除(`IN_MOVE_SELF`)时，会停止文件的监控、读取、并关闭 fd。



上面未分析的，包括 rotate (rename) 的场景。



细心如你，可能会担心上面的协程会否同时读取文件或更新状态，引动竞态（多线程）问题。这个已经由下面的事件事件驱动与协程框架解决了。



## 事件驱动与协程

> 以下例子场景，使用了 Fluent Bit 1.99 与其 `Tail Input`  + `Http Output` 



```bash
$ top -H -p $(pgrep fluent-bit )

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                                                                                                                                                                                              
    27 226099    20   0  417804  67096   9240 S 0.000 0.069   0:02.13 fluent-bit     
    35 226099    20   0  417804  67096   9240 S 0.000 0.069   1:16.61 flb-pipeline   
    37 226099    20   0  417804  67096   9240 S 0.000 0.069   0:06.69 flb-logger     
    45 226099    20   0  417804  67096   9240 S 0.000 0.069   0:11.58 flb-out-http.0-
    46 226099    20   0  417804  67096   9240 S 0.000 0.069   0:11.70 flb-out-http.0-
    47 226099    20   0  417804  67096   9240 S 0.000 0.069   0:00.00 monkey: server 
    48 226099    20   0  417804  67096   9240 S 0.000 0.069   0:03.17 monkey: clock
    49 226099    20   0  417804  67096   9240 S 0.000 0.069   0:23.82 monkey: wrk/0
```



用 `top -H` 可以看到 fluent bit 进程的原生线程列表。`PID`列即系线程的 id，而最少的线程 PID 同时作为进程的 PID。其中比较有意思的是 `TIME+` 字段。这表示花在这个线程上的 CPU 计算时间。 以下是推测：

* `flb-pipeline `: 日志处理与输出
* `monkey: wrk/0`: 日志文件读取



#### 什么是 monkey ?

> [https://github.com/monkey/monkey](https://github.com/monkey/monkey)
>
> [Monkey](http://monkey-project.com/) is a fast and lightweight Web Server for Linux. It has been designed to be very scalable with low memory and CPU consumption, the perfect solution for Embedded Linux and high end production environments.
>
> Besides the common features as HTTP server, it expose a flexible C API which aims to behave as a fully HTTP development framework, so it can be extended as desired through the plugins interface.
>
> For more details please refer to the [official documentation](http://monkey-project.com/documentation/).

Fluent Bit 中，主要是用了其协程和事件驱动封装的功能。协程的实现设计上有一点点类似 Golang。上图的线程名中 `monkey: wrk/0` 。可见，是在计算量大时，可以为协程增加必要的线程来支持计算。从代码看，似乎协程的换出点(schedule) 是在 `file descriptor(fd)` 的读写点上，实现上 monkey 似乎是使用了 epoll 去多路复用 fd 集合。协程间的同步通讯由 linux 的匿名 pipe + epoll 完成。即，线程事实上是等待在一个多路复用的 epoll 事件上。



查看各线程的内核 stack:

```
 root@root-mylab-worker006:/proc/27/task> sudo cat ./35/stack 
[<0>] ep_poll+0x3d4/0x4d0
[<0>] do_epoll_wait+0xab/0xc0
[<0>] __x64_sys_epoll_wait+0x1a/0x20
[<0>] do_syscall_64+0x5b/0x1e0
[<0>] entry_SYSCALL_64_after_hwframe+0x44/0xa9

root@root-mylab-worker006:/proc/27/task> sudo cat ./49/stack 
[<0>] ep_poll+0x3d4/0x4d0
[<0>] do_epoll_wait+0xab/0xc0
[<0>] __x64_sys_epoll_wait+0x1a/0x20
[<0>] do_syscall_64+0x5b/0x1e0
[<0>] entry_SYSCALL_64_after_hwframe+0x44/0xa9

root@root-mylab-worker006:/proc/27/task> sudo cat ./48/stack 
[<0>] hrtimer_nanosleep+0x9a/0x140
[<0>] common_nsleep+0x33/0x50
[<0>] __x64_sys_clock_nanosleep+0xc4/0x120
[<0>] do_syscall_64+0x5b/0x1e0
[<0>] entry_SYSCALL_64_after_hwframe+0x44/0xa9
```



### 文件 fd 即事件源



如果你足够好奇，可以看看进程的 fd 列表：

```
bash-4.4$ cd /proc/27
bash-4.4$ cd fd
bash-4.4$ ls -l
total 0
lr-x------ 1 226099 226099 64 Dec 13 19:39 0 -> /dev/null
l-wx------ 1 226099 226099 64 Dec 13 19:39 1 -> 'pipe:[1066519386]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 10 -> 'pipe:[1066519390]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 100 -> anon_inode:inotify
lr-x------ 1 226099 226099 64 Dec 13 19:39 101 -> 'pipe:[1066516725]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 102 -> 'pipe:[1066516725]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 103 -> 'pipe:[1066516726]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 104 -> 'pipe:[1066516726]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 105 -> 'pipe:[1066516727]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 106 -> 'pipe:[1066516727]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 107 -> /var/logstash/db/myapp_mysub_pv.db
lr-x------ 1 226099 226099 64 Dec 13 19:39 108 -> anon_inode:inotify <---- intofiy
lr-x------ 1 226099 226099 64 Dec 13 19:39 109 -> 'pipe:[1066516745]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 11 -> 'anon_inode:[eventpoll]' <----- epoll
l-wx------ 1 226099 226099 64 Dec 13 19:39 110 -> 'pipe:[1066516745]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 111 -> 'pipe:[1066516746]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 112 -> 'pipe:[1066516746]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 113 -> 'pipe:[1066516747]'
l-wx------ 1 226099 226099 64 Dec 13 19:39 114 -> 'pipe:[1066516747]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 115 -> /var/logstash/db/myapp_mysub_pv_outbound.db
...
lrwx------ 1 226099 226099 64 Dec 13 19:39 363 -> /var/logstash/db/myapp_mysub2.db-wal
lrwx------ 1 226099 226099 64 Dec 13 19:39 364 -> /var/logstash/db/myapp_mysub2.db-shm
...
lrwx------ 1 226099 226099 64 Dec 13 19:39 485 -> 'anon_inode:[timerfd]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 486 -> 'anon_inode:[timerfd]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 487 -> 'anon_inode:[timerfd]'
lrwx------ 1 226099 226099 64 Dec 13 19:39 488 -> 'anon_inode:[timerfd]'
...
lrwx------ 1 226099 226099 64 Dec 13 19:39 681 -> 'socket:[1067595164]'
lr-x------ 1 226099 226099 64 Dec 13 19:39 685 -> /var/logstash/mylog/Txlog.mylogB_0.log.2022-12-13-18
lr-x------ 1 226099 226099 64 Dec 13 19:39 686 -> /var/logstash/mylog/Txlog.mylogA_0.log.2022-12-13-19
```



## 关于重试

Fluent Bit 的 Output 插件，在尝试投递一个 Chunk 日志后，都会告诉引擎，

上过生产环境战场的程序员，都明白这两件事的重要性和难度，并深谙此道：

- timeout
- retry

这里只说 retry。retry 多了（甚至无限），会出现因一个（或一批）不可修复的 item 而卡数据或业务流的情况。我就亲眼无过没处理好 retry 卡了大量订单的情况（因一字段超过了接收方的最大长度）。

如果你不想因一棵坏的树而损失了一个森林，选择什么时候跳过(skip)是关键。



### Retryable error / Non-retryable error

对于实现上，如果我们可以清楚，什么错误是值得重试的，什么不值得



