---
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


### 物理内存整理
https://www.kernel.org/doc/html/latest/admin-guide/mm/concepts.html#compaction

##### [Compaction](https://www.kernel.org/doc/html/latest/admin-guide/mm/concepts.html#id8)[](https://www.kernel.org/doc/html/latest/admin-guide/mm/concepts.html#compaction "Permalink to this headline")

As the system runs, tasks allocate and free the memory and it becomes fragmented. Although with virtual memory it is possible to present scattered physical pages as virtually contiguous range, sometimes it is necessary to allocate large physically contiguous memory areas. Such need may arise, for instance, when a device driver requires a large buffer for DMA, or when THP allocates a huge page. Memory compaction addresses the fragmentation issue. This mechanism moves occupied pages from the lower part of a memory zone to free pages in the upper part of the zone. When a compaction scan is finished free pages are grouped together at the beginning of the zone and allocations of large physically contiguous areas become possible.


Like reclaim, the compaction may happen asynchronously in the `kcompactd` daemon or **synchronously** as a result of a memory allocation request.

