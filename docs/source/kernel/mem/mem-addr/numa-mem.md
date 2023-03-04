---
title: "NUMA - memory"
date: 2021-03-01T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- kernel
- kernel-mem
- kernel-mem-address
---



> [Linux Kernel Programming - Kaiwan N Billimoria] :



## Zone

CPU Socket -> NUMA Node -> Zone

![image-20220709115248630](numa-mem.assets/image-20220709115248630.png)



## page frames

![image-20220709115509916](numa-mem.assets/image-20220709115509916.png)



```log
$ cat /proc/buddyinfo
Node 0, zone
DMA
3
3
Node 0, zone
DMA32 31306
0
Node 0, zone Normal 49135
0
$
```

