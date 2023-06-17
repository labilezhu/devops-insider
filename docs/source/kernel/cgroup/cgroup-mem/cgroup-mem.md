---
title: "cgroup memory"
date: 2021-02-26T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- kernel
- kernel-mem
- cgroup
---

# from [Linux Kernel Programming - Kaiwan N Billimoria]

## Control groups and memory





# from kernel.org/doc


## Accounting details


> https://www.kernel.org/doc/Documentation/cgroup-v1/memory.txt


All mapped anon pages (RSS) and cache pages (Page Cache) are accounted.

### Private RSS

RSS pages are accounted at page_fault unless they've already been accounted
for earlier.


### Share Page
Shared pages are accounted on the basis of the first touch approach. The
cgroup that first touches a page is accounted for the page. The principle
behind this approach is that a cgroup that aggressively uses a shared
page will eventually get charged for it (once it is uncharged from
the cgroup that brought it in -- this will happen on memory pressure).

### Kernel Memory resources accounted

* stack pages: every process consumes some stack pages. By accounting into
kernel memory, we prevent new processes from being created when the kernel
memory usage is too high.

* slab pages: pages allocated by the SLAB or SLUB allocator are tracked. A copy
of each kmem_cache is created every time the cache is touched by the first time
from inside the memcg. The creation is done lazily, so some objects can still be
skipped while the cache is being created. All objects in a slab page should
belong to the same memcg. This only fails to hold when a task is migrated to a
different memcg during the page allocation by the cache.

* sockets memory pressure: some sockets protocols have memory pressure
thresholds. The Memory Controller allows them to be controlled individually
per cgroup, instead of globally.

* tcp memory pressure: sockets memory pressure for the tcp protocol.



## Reclaim

Each cgroup maintains a per cgroup LRU which has the same structure as
global VM. When a cgroup goes over its limit, we first try
to reclaim memory from the cgroup so as to make space for the new
pages that the cgroup has touched. If the reclaim is unsuccessful,
an OOM routine is invoked to select and kill the bulkiest task in the
cgroup. (See 10. OOM Control below.)

## Troubleshooting

### stat file

#### memory.stat

```
memory.stat file includes following statistics

# per-memory cgroup local status
cache		- # of bytes of page cache memory.
rss		- # of bytes of anonymous and swap cache memory (includes
		transparent hugepages).
rss_huge	- # of bytes of anonymous transparent hugepages.
mapped_file	- # of bytes of mapped file (includes tmpfs/shmem)
pgpgin		- # of charging events to the memory cgroup. The charging
		event happens each time a page is accounted as either mapped
		anon page(RSS) or cache page(Page Cache) to the cgroup.
pgpgout		- # of uncharging events to the memory cgroup. The uncharging
		event happens each time a page is unaccounted from the cgroup.
swap		- # of bytes of swap usage
dirty		- # of bytes that are waiting to get written back to the disk.
writeback	- # of bytes of file/anon cache that are queued for syncing to
		disk.
inactive_anon	- # of bytes of anonymous and swap cache memory on inactive
		LRU list.
active_anon	- # of bytes of anonymous and swap cache memory on active
		LRU list.
inactive_file	- # of bytes of file-backed memory on inactive LRU list.
active_file	- # of bytes of file-backed memory on active LRU list.
unevictable	- # of bytes of memory that cannot be reclaimed (mlocked etc)
```
#### memory.usage_in_bytes

usage_in_bytes

For efficiency, as other kernel components, memory cgroup uses some optimization
to avoid unnecessary cacheline false sharing. usage_in_bytes is affected by the
method and doesn't show 'exact' value of memory (and swap) usage, it's a fuzz
value for efficient access. (Of course, when necessary, it's synchronized.)
**If you want to know more exact memory usage, you should use RSS+CACHE(+SWAP) value in memory.stat**

#### memory.numa_stat
```
This is similar to numa_maps but operates on a per-memcg basis.  This is
useful for providing visibility into the numa locality information within
an memcg since the pages are allowed to be allocated from any physical
node.  One of the use cases is evaluating application performance by
combining this information with the application's CPU allocation.

Each memcg's numa_stat file includes "total", "file", "anon" and "unevictable"
per-node page counts including "hierarchical_<counter>" which sums up all
hierarchical children's values in addition to the memcg's own value.

The output format of memory.numa_stat is:

total=<total pages> N0=<node 0 pages> N1=<node 1 pages> ...
file=<total file pages> N0=<node 0 pages> N1=<node 1 pages> ...
anon=<total anon pages> N0=<node 0 pages> N1=<node 1 pages> ...
unevictable=<total anon pages> N0=<node 0 pages> N1=<node 1 pages> ...
hierarchical_<counter>=<counter pages> N0=<node 0 pages> N1=<node 1 pages> ...

The "total" count is sum of file + anon + unevictable.
```

#### memory.failcnt 
cat ./memory.failcnt 
19369

A memory cgroup provides memory.failcnt and memory.memsw.failcnt files.
This failcnt(== failure count) shows the number of times that a usage counter
hit its limit. When a memory cgroup hits a limit, failcnt increases and
memory under it will be reclaimed.

You can reset failcnt by writing 0 to failcnt file.
```bash
# echo 0 > .../memory.failcnt
```

> https://srvaroa.github.io/jvm/kubernetes/memory/docker/oomkiller/2019/05/29/k8s-and-java.html

### Page cache
> https://engineering.linkedin.com/blog/2016/08/don_t-let-linux-control-groups-uncontrolled

* Page cache usage by apps is counted towards a cgroup’s memory limit, and anonymous memory usage can steal page cache for the same cgroup

Insufficient page cache can result in lowered application performance, as more disk IO is expected.

> https://medium.com/@eng.mohamed.m.saeed/memory-working-set-vs-memory-rss-in-kubernetes-which-one-you-should-monitor-8ef77bf0acee

