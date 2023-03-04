---
title: "Kernel - Memory Area"
date: 2021-02-26T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- kernel
- kernel-mem

---

## Memory Area Management

使用 `buddy system algorithm`来分配大块内存是合理的，但小块内存就会做成空间浪费。

### Slab Allocator

> [Linux Kernel Programming - Kaiwan N Billimoria] :
>
> ![image-20220709121036876](index.assets/image-20220709121036876.png)




在 `buddy system algorithm`之上做一个内存分配算法会很低效。一个更好的方法是：

* 以不同数据结构分组，一次初始化，多次重用数据结构的内存块
* 内核经常频繁申请相同的数据结构，像 process descriptor

Slab Allocator 把对象按不同的数据结构类型分组为 Cache。Cache 再分为 Slab。一个 Slab必须由连续的 Page 组成。每个 Slab 又包含已分配和未分配的对象：

![image-20210301164300244](index.assets/image-20210301164300244.png)



## Cache Descriptor

每个 Cache 对应一个 kmem_cache_t 数据结构。

#### Slab Descriptor

![image-20210301170710370](index.assets/image-20210301170710370.png)



Cache 与 Slab 的关系：

![image-20210301171051953](index.assets/image-20210301171051953.png)



## 非连续 Page  的内存管理

![image-20210301172723692](index.assets/image-20210301172723692.png)

相关函数：

*  vmalloc( )
* vfree( )





## 参考



[Understanding The Linux Kernel 3rd Edition]

