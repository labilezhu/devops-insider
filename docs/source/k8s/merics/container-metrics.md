# Container Metrics

## Memory



K8s 自带容器指标数据源是来自 [kubelet 中运行的 cAdvisor 模块](https://kubernetes.io/docs/concepts/cluster-administration/system-metrics/#:~:text=kubelet%20collects%20accelerator%20metrics%20through%20cAdvisor) 的。 

而 cAdvisor 的官方 Metric 说明文档在这：[Monitoring cAdvisor with Prometheus](https://github.com/google/cadvisor/blob/master/docs/storage/prometheus.md#:~:text=container_memory_usage_bytes) 。这个官方文档是写得太简单了，简单到不太适合问题定位……

好在，高手在民间：[Out-of-memory (OOM) in Kubernetes – Part 3: Memory metrics sources and tools to collect them](https://mihai-albert.com/2022/02/13/out-of-memory-oom-in-kubernetes-part-3-memory-metrics-sources-and-tools-to-collect-them/):

| **cAdvisor metric**                      | **Source OS metric(s)**                                      | **Explanation of source OS metric(s)**                       | **What does the metric mean?**                               |
| ---------------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| **`container_memory_cache`**             | **`total_cache`** value in the **`memory.stat`** file inside the container’s cgroup directory folder | *number of bytes of page cache memory*                       | Size of memory used by the cache that’s automatically populated when reading/writing files |
| **`container_memory_rss`**               | **`total_rss`** value in the **`memory.stat`** file inside the container’s cgroup directory folder | *number of bytes of anonymous and swap cache memory (includes transparent hugepages). […]This should not be confused with the true ‘resident set size’ or the amount of physical memory used by the cgroup. ‘rss + mapped_file’ will give you resident set size of cgroup”* | Size of memory not used for mapping files from the disk      |
| **`container_memory_mapped_file`**       | **`total_mapped_file`** value in the **`memory.stat`** file inside the container’s cgroup directory folder | *number of bytes of mapped file (includes tmpfs/shmem)*      | Size of memory that’s used for mapping files                 |
| **`container_memory_swap`**              | **`total_swap`** value in the **`memory.stat`** file inside the container’s cgroup directory folder | *number of bytes of swap usage*                              |                                                              |
| **`container_memory_failcnt`**           | The value inside the **`memory.failcnt`** file               | *shows the number of times that a usage counter hit its limit* |                                                              |
| **`container_memory_usage_bytes`**       | The value inside the **`memory.usage_in_bytes`** file        | *doesn’t show ‘exact’ value of memory (and swap) usage, it’s a fuzz value for efficient access. (Of course, when necessary, it’s synchronized.) If you want to know more exact memory usage, you should use RSS+CACHE(+SWAP) value in memory.stat* | Size of overall memory used, regardless if it’s for mapping from disk or just allocating |
| **`container_memory_max_usage_bytes`**   | The value inside the **`memory.max_usage_in_bytes`** file    | *max memory usage recorded*                                  |                                                              |
| **`container_memory_working_set_bytes`** | Deduct **`inactive_file`** inside the **`memory.stat `**file from the value inside the **`memory.usage_in_bytes`** file. If result is negative then use 0 | **`inactive_file`**: *number of bytes of file-backed memory on inactive LRU list* **`usage_in_bytes`**: *doesn’t show ‘exact’ value of memory (and swap) usage, it’s a fuzz value for efficient access. (Of course, when necessary, it’s synchronized.) If you want to know more exact memory usage, you should use RSS+CACHE(+SWAP) value in memory.stat* | A heuristic for the minimum size of memory required for the app to work.<br />*the amount of memory in-use that cannot be freed under memory pressure[…] It includes all anonymous (non-file-backed) memory since Kubernetes does not support swap. The metric typically also includes some cached (file-backed) memory, because the host OS cannot always reclaim such pages*. See the cAdvisor table for the formula containing base OS metrics |

*表：CAdvisor 的指标和来源*



如果上面的描述还不足以满足你的好奇心，那么这里有更多：

- https://jpetazzo.github.io/2013/10/08/docker-containers-metrics/



### 常被误解的 K8s 指标

#### container_memory_usage_bytes

> [A Deep Dive into Kubernetes Metrics — Part 3 Container Resource Metrics](https://blog.freshtracks.io/a-deep-dive-into-kubernetes-metrics-part-3-container-resource-metrics-361c5ee46e66#fromHistory:~:text=easily%20tracked%20with-,container_memory_usage_bytes,-%2C%20however%2C%20this%20metric)
>
> You might think that memory utilization is easily tracked with `container_memory_usage_bytes`, however, this metric also includes cached (think filesystem cache) items that can be evicted under memory pressure. The better metric is `container_memory_working_set_bytes` as this is what the OOM killer is watching for.

#### container_memory_working_set_bytes

> [Memory usage discrepancy: cgroup memory.usage_in_bytes vs. RSS inside docker container](https://stackoverflow.com/questions/50865763/memory-usage-discrepancy-cgroup-memory-usage-in-bytes-vs-rss-inside-docker-con)
>
> `container_memory_working_set_bytes` = `container_memory_usage_bytes` - `total_inactive_file` (from /sys/fs/cgroup/memory/memory.stat), this is calculated in cAdvisor and is <= `container_memory_usage_bytes`

#### kubectl top

> [Memory usage discrepancy: cgroup memory.usage_in_bytes vs. RSS inside docker container](https://stackoverflow.com/questions/50865763/memory-usage-discrepancy-cgroup-memory-usage-in-bytes-vs-rss-inside-docker-con)
>
> when you use the `kubectl top pods` command, you get the value of `container_memory_working_set_bytes` not `container_memory_usage_bytes` metric.

#### container_memory_cache 与 container_memory_mapped_file 的关系

> [Out-of-memory (OOM) in Kubernetes – Part 3: Memory metrics sources and tools to collect them](https://mihai-albert.com/2022/02/13/out-of-memory-oom-in-kubernetes-part-3-memory-metrics-sources-and-tools-to-collect-them/):
>
> Notice the “page cache” term on the definition of the `container_memory_cache` metric. In Linux the page cache is “*used to cache the content of files as IO is performed upon them*” as per the “`Linux Kernel Programming`” book by Kaiwan N Billimoria(本文作者注：这本书我看过，是我看到的，最近最好解理的内核图书). You might be tempted as such to think that `container_memory_mapped_file` pretty much refers to the same thing, but that’s actually just a subset: e.g. a file can be mapped in memory (whole or parts of it) or it can be read in blocks, but the page cache will include data coming from either way of accessing that file. See https://stackoverflow.com/questions/258091/when-should-i-use-mmap-for-file-access for more info.





#### 什么 metric 才是 OOM Kill 相关

> [Memory usage discrepancy: cgroup memory.usage_in_bytes vs. RSS inside docker container](https://stackoverflow.com/questions/50865763/memory-usage-discrepancy-cgroup-memory-usage-in-bytes-vs-rss-inside-docker-con)
>
> It is also worth to mention that when the value of `container_memory_usage_bytes` reaches to the limits, your pod will NOT get oom-killed. BUT if `container_memory_working_set_bytes` or `container_memory_rss` reached to the limits, the pod will be killed.



### 参考

- [https://blog.mygraphql.com/zh/posts/low-tec/kernel/cgroup-mem/](https://blog.mygraphql.com/zh/posts/low-tec/kernel/cgroup-mem/)
- 

