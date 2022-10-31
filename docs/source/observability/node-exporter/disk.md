# Disk

> [https://www.robustperception.io/mapping-iostat-to-the-node-exporters-node_disk_-metrics/](https://www.robustperception.io/mapping-iostat-to-the-node-exporters-node_disk_-metrics/)

## Mapping iostat to the node exporter’s node\_disk\_\* metrics

The node exporter and tools like iostat and sar use the same core data, but how do they relate to each other?

Prometheus metric names tend to tie pretty directly to a raw data source, so `node_disk_reads_completed_total` is the number of reads completed from a given disk device given by the 1st field of [/proc/diskstats](https://www.kernel.org/doc/Documentation/iostats.txt). It's reasonably obvious what it means just from the name. `iostat -x` has output like:

```
Device: rrqm/s wrqm/s  r/s   w/s rkB/s  wkB/s avgrq-sz avgqu-sz await r\_await w\_await svctm %util
sda       0.00  8.00  0.00 15.00  0.00 342.00    45.60     0.04  2.40    0.00    2.40  0.40  0.60
```

Here the `r/s` is the number of reads per second calculated from the previous measurement iostat made (or since boot for the first one). The equivalent is `rate(node_disk_reads_completed_total[5m])` in PromQL. Similarly with `w/s` and `node_disk_writes_completed_total`, `rrqm/s` and `node_disk_reads_merged_total`, and `wrqm/s` and `node_disk_writes_merged_total`.

For bandwidth, `iostat` will report in kilobytes by default. The kernel reports this in 512-sectors (irrelevant of the sector size of the underlying device), and Prometheus uses bytes as standard. So `rkB/s` and `wkB/s` would be `rate(node_disk_read_bytes_total[5m])` and `rate(node_disk_written_bytes_total[5m])` only 1024 times bigger.

`avgrq-sz` is the average size of each request, combining both read and write. It's calculated by iostat by dividing the bytes by the operations, so  
`(rate(node_disk_read_bytes_total[5m]) + rate(node_disk_written_bytes_total[5m]))  
/  
(rate(node_disk_reads_completed_total[5m]) + rate(node_disk_writes_completed_total[5m]))`  
and once again in bytes rather than kilobytes. Personally I'd rather view reads and writes separately.

`avgqu-sz` is simpler, the average queue length. This is based on field 11, which gives us `rate(node_disk_io_time_weighted_seconds_total[5m])`.

`r_await` and `w_await` are how long read and write requests took on average, so for reads that's `rate(node_disk_read_time_seconds_total[5m]) / rate(node_disk_reads_completed_total[5m])`and similarly for writes. `await` is both combined, so you can add and then divide if you want it.

`%util` is utilisation as a percentage, `rate(node_disk_io_time_seconds_total[5m])` will produce the same as a ratio which is more standard in Prometheus. `svctm` is deprecated, but it'd be the IO time divided by the sum of the reads and writes completed.

There one other notable metric which `iostat` doesn't expose which is field 9, `node_disk_io_now` the number of IOs in progress. Newer kernels will also expose [discard stats](https://lwn.net/Articles/756667/), useful for SSDs.
