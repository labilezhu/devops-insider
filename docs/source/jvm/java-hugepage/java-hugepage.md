
> https://ionutbalosin.com/2020/01/hotspot-jvm-performance-tuning-guidelines/

| **-XX:+UseLargePages** (default false) |
| --- |
| Enable the use of large page memory. The goal of the large page support is to optimize processor Translation-Lookaside Buffers (TLB) and hence increase performance.Large pages might be suitable for intensive memory applications with large contiguous memory accesses.Large pages might not be suitable for (i) short-lived applications with a small working set or for (ii) applications with a large but sparsely used heap.**Note**: consider enabling large pages when the number of TLB misses and TLB Page walk take a significant amount of time (i.e. *dtlb_load_misses_** CPU counters). |

