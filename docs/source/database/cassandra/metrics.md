# Metrics



## Thread pool and read/write latency statistics

> https://docs.datastax.com/en/cassandra-oss/3.x/cassandra/operations/opsThreadPoolStats.html

Cassandra maintains distinct thread pools for different stages of execution. Each of the thread pools provide statistics on the number of tasks that are active, pending, and completed. Trends on these pools for increases in the pending tasks column indicate when to add additional capacity. <mark>After a baseline is established, configure alarms for any increases above normal in the pending tasks column.</mark> Use [nodetool tpstats](https://docs.datastax.com/en/cassandra-oss/3.x/cassandra/tools/toolsTPstats.html) on the command line to view the thread pool details shown in the following table.

| Thread Pool                  | Description                                                  |
| ---------------------------- | ------------------------------------------------------------ |
| AntiEntropyStage             | Tasks related to repair                                      |
| CacheCleanupExecutor         | Tasks related to cache maintenance (counter cache, row cache) |
| CompactionExecutor           | Tasks related to compaction                                  |
| CounterMutationStage         | Tasks related to leading counter writes                      |
| GossipStage                  | Tasks related to the gossip protocol                         |
| HintsDispatcher              | Tasks related to sending hints                               |
| InternalResponseStage        | Tasks related to miscellaneous internal task responses       |
| MemtableFlushWriter          | Tasks related to flushing memtables                          |
| MemtablePostFlush            | Tasks related to maintenance after memtable flush completion |
| MemtableReclaimMemory        | Tasks related to reclaiming memtable memory                  |
| MigrationStage               | Tasks related to schema maintenance                          |
| MiscStage                    | Tasks related to miscellaneous tasks, including snapshots and removing hosts |
| MutationStage                | Tasks related to writes                                      |
| Native-Transport-Requests    | Tasks related to client requests from CQL                    |
| PendingRangeCalculator       | Tasks related to recalculating range ownership after bootstraps/decommissions |
| PerDiskMemtableFlushWriter_* | Tasks related to flushing memtables to a given disk          |
| ReadRepairStage              | Tasks related to performing read repairs                     |
| ReadStage                    | Tasks related to reads                                       |
| RequestResponseStage         | Tasks for callbacks from intra-node requests                 |
| Sampler                      | Tasks related to sampling statistics                         |
| SecondaryIndexManagement     | Tasks related to secondary index maintenance                 |
| ValidationExecutor           | Tasks related to validation compactions                      |
| ViewMutationStage            | Tasks related to maintaining materialized views              |


### Metrics of a thread pool
> https://www.eginnovations.com/documentation/Cassandra-Database/Cassandra-Thread-Pools-Test.htm

| Measurement           | Description                                                  | Measurement Unit | Interpretation                                               |
| :-------------------- | :----------------------------------------------------------- | :--------------- | :----------------------------------------------------------- |
| Active tasks          | Indicates the number of active tasks in this thread pool.    | Number           | Compare the value of this measure across the thread pools to figure out the thread pool that contains the maximum number of threads that are active. |
| Completed tasks       | Indicates the rate at which tasks were completed in this thread pool. | Tasks/sec        | Compare the value of this measure across the thread pools to figure out the thread pool that has completed the maximum number of tasks per second. |
| Current blocked tasks | Indicates the number of tasks that are currently blocked in this thread pool. | Number           | A high value for this measure indicates that there are no more threads in the thread pool to service the tasks. To avoid the tasks from being blocked, administrators can calibrate the thread pool to accommodate enough threads to service the tasks. |
| Pending tasks         | Indicates the number of tasks that are pending in this thread pool. | Number           | Compare the value across the thread pools to identify the thread pool on which the maximum number of tasks are pending.A sudden/gradual increase in the measure is an indication for the administrators to add additional threads to the thread pool. |
| Total blocked tasks   | Indicates the rate at which tasks were blocked in this thread pool during the last measurement period. | Tasks/sec        | A low value is desired for this measure.                     |


> https://www.instaclustr.com/support/documentation/cassandra/cassandra-monitoring/thread-pool-metrics/
## Dropped Messages

The Dropped Messages metric represents the total number of dropped messages from all stages in the SEDA.

> https://cassandra.apache.org/doc/3.11/cassandra/operating/metrics.html#dropped-metrics

Metrics specific to tracking dropped messages for different types of requests. Dropped writes are stored and retried by `Hinted Handoff`

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.DroppedMessage.<MetricName>.<Type>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=DroppedMessage scope=<Type> name=<MetricName>`

| Name                    | Type  | Description                       |
| :---------------------- | :---- | :-------------------------------- |
| CrossNodeDroppedLatency | Timer | The dropped latency across nodes. |
| InternalDroppedLatency  | Timer | The dropped latency within node.  |
| Dropped                 | Meter | Number of dropped messages.       |

The different types of messages tracked are:

| Name             | Description                                  |
| :--------------- | :------------------------------------------- |
| BATCH_STORE      | Batchlog write                               |
| BATCH_REMOVE     | Batchlog cleanup (after succesfully applied) |
| COUNTER_MUTATION | Counter writes                               |
| HINT             | Hint replay                                  |
| MUTATION         | Regular writes                               |
| READ             | Regular reads                                |
| READ_REPAIR      | Read repair                                  |
| PAGED_SLICE      | Paged read                                   |
| RANGE_SLICE      | Token range read                             |
| REQUEST_RESPONSE | RPC Callbacks                                |
| _TRACE           | Tracing writes                               |

## Read/Write latency metrics

Cassandra tracks latency (averages and totals) of read, write, and slicing operations at the server level through StorageProxyMBean.





## Table Metrics

> https://murukeshm.github.io/cassandra/3.10/operating/metrics.html#table-metrics

Each table in Cassandra has metrics responsible for tracking its state and performance.

The metric names are all appended with the specific `Keyspace` and `Table` name.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Table.<MetricName>.<Keyspace>.<Table>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Table keyspace=<Keyspace> scope=<Table> name=<MetricName>`

Note

There is a special table called ‘`all`‘ without a keyspace. This represents the aggregation of metrics across **all** tables and keyspaces on the node.

| Name                                 | Type           | Description                                                  |
| ------------------------------------ | -------------- | ------------------------------------------------------------ |
| MemtableOnHeapSize                   | Gauge<Long>    | Total amount of data stored in the memtable that resides **on**-heap, including column related overhead and partitions overwritten. |
| MemtableOffHeapSize                  | Gauge<Long>    | Total amount of data stored in the memtable that resides **off**-heap, including column related overhead and partitions overwritten. |
| MemtableLiveDataSize                 | Gauge<Long>    | Total amount of live data stored in the memtable, excluding any data structure overhead. |
| AllMemtablesOnHeapSize               | Gauge<Long>    | Total amount of data stored in the memtables (2i and pending flush memtables included) that resides **on**-heap. |
| AllMemtablesOffHeapSize              | Gauge<Long>    | Total amount of data stored in the memtables (2i and pending flush memtables included) that resides **off**-heap. |
| AllMemtablesLiveDataSize             | Gauge<Long>    | Total amount of live data stored in the memtables (2i and pending flush memtables included) that resides off-heap, excluding any data structure overhead. |
| MemtableColumnsCount                 | Gauge<Long>    | Total number of columns present in the memtable.             |
| MemtableSwitchCount                  | Counter        | Number of times flush has resulted in the memtable being switched out. |
| CompressionRatio                     | Gauge<Double>  | Current compression ratio for all SSTables.                  |
| EstimatedPartitionSizeHistogram      | Gauge<long[]>  | Histogram of estimated partition size (in bytes).            |
| EstimatedPartitionCount              | Gauge<Long>    | Approximate number of keys in table.                         |
| EstimatedColumnCountHistogram        | Gauge<long[]>  | Histogram of estimated number of columns.                    |
| SSTablesPerReadHistogram             | Histogram      | Histogram of the number of sstable data files accessed per read. |
| ReadLatency                          | Latency        | Local read latency for this table.                           |
| RangeLatency                         | Latency        | Local range scan latency for this table.                     |
| WriteLatency                         | Latency        | Local write latency for this table.                          |
| CoordinatorReadLatency               | Timer          | Coordinator read latency for this table.                     |
| CoordinatorScanLatency               | Timer          | Coordinator range scan latency for this table.               |
| PendingFlushes                       | Counter        | Estimated number of flush tasks pending for this table.      |
| BytesFlushed                         | Counter        | Total number of bytes flushed since server [re]start.        |
| CompactionBytesWritten               | Counter        | Total number of bytes written by compaction since server [re]start. |
| PendingCompactions                   | Gauge<Integer> | Estimate of number of pending compactions for this table.    |
| LiveSSTableCount                     | Gauge<Integer> | Number of SSTables on disk for this table.                   |
| LiveDiskSpaceUsed                    | Counter        | Disk space used by SSTables belonging to this table (in bytes). |
| TotalDiskSpaceUsed                   | Counter        | Total disk space used by SSTables belonging to this table, including obsolete ones waiting to be GC’d. |
| MinPartitionSize                     | Gauge<Long>    | Size of the smallest compacted partition (in bytes).         |
| MaxPartitionSize                     | Gauge<Long>    | Size of the largest compacted partition (in bytes).          |
| MeanPartitionSize                    | Gauge<Long>    | Size of the average compacted partition (in bytes).          |
| BloomFilterFalsePositives            | Gauge<Long>    | Number of false positives on table’s bloom filter.           |
| BloomFilterFalseRatio                | Gauge<Double>  | False positive ratio of table’s bloom filter.                |
| BloomFilterDiskSpaceUsed             | Gauge<Long>    | Disk space used by bloom filter (in bytes).                  |
| BloomFilterOffHeapMemoryUsed         | Gauge<Long>    | Off-heap memory used by bloom filter.                        |
| IndexSummaryOffHeapMemoryUsed        | Gauge<Long>    | Off-heap memory used by index summary.                       |
| CompressionMetadataOffHeapMemoryUsed | Gauge<Long>    | Off-heap memory used by compression meta data.               |
| KeyCacheHitRate                      | Gauge<Double>  | Key cache hit rate for this table.                           |
| TombstoneScannedHistogram            | Histogram      | Histogram of tombstones scanned in queries on this table.    |
| LiveScannedHistogram                 | Histogram      | Histogram of live cells scanned in queries on this table.    |
| ColUpdateTimeDeltaHistogram          | Histogram      | Histogram of column update time delta on this table.         |
| ViewLockAcquireTime                  | Timer          | Time taken acquiring a partition lock for materialized view updates on this table. |
| ViewReadTime                         | Timer          | Time taken during the local read of a materialized view update. |
| TrueSnapshotsSize                    | Gauge<Long>    | Disk space used by snapshots of this table including all SSTable components. |
| RowCacheHitOutOfRange                | Counter        | Number of table row cache hits that do not satisfy the query filter, thus went to disk. |
| RowCacheHit                          | Counter        | Number of table row cache hits.                              |
| RowCacheMiss                         | Counter        | Number of table row cache misses.                            |
| CasPrepare                           | Latency        | Latency of paxos prepare round.                              |
| CasPropose                           | Latency        | Latency of paxos propose round.                              |
| CasCommit                            | Latency        | Latency of paxos commit round.                               |
| PercentRepaired                      | Gauge<Double>  | Percent of table data that is repaired on disk.              |
| SpeculativeRetries                   | Counter        | Number of times speculative retries were sent for this table. |
| WaitingOnFreeMemtableSpace           | Histogram      | Histogram of time spent waiting for free memtable space, either on- or off-heap. |
| DroppedMutations                     | Counter        | Number of dropped mutations on this table.                   |

## Keyspace Metrics

Each keyspace in Cassandra has metrics responsible for tracking its state and performance.

These metrics are the same as the `Table Metrics` above, only they are aggregated at the Keyspace level.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.keyspace.<MetricName>.<Keyspace>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Keyspace scope=<Keyspace> name=<MetricName>`



## Client Request Metrics

> https://murukeshm.github.io/cassandra/3.10/operating/metrics.html#client-request-metrics

Client requests have their own set of metrics that encapsulate the work happening at coordinator level.

Different types of client requests are broken down by `RequestType`.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.ClientRequest.<MetricName>.<RequestType>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=ClientRequest scope=<RequestType> name=<MetricName>`

| RequestType: | CASRead                                                      |
| :----------- | ------------------------------------------------------------ |
| Description: | Metrics related to transactional read requests.              |
| Metrics:     | NameTypeDescriptionTimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of transaction failures encountered. LatencyTransaction read latency.UnavailablesCounterNumber of unavailable exceptions encountered.UnfinishedCommitCounterNumber of transactions that were committed on read.ConditionNotMetCounterNumber of transaction preconditions did not match current values.ContentionHistogramHistogramHow many contended reads were encountered |
| RequestType: | CASWrite                                                     |
| Description: | Metrics related to transactional write requests.             |
| Metrics:     | NameTypeDescriptionTimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of transaction failures encountered. LatencyTransaction write latency.UnfinishedCommitCounterNumber of transactions that were committed on write.ConditionNotMetCounterNumber of transaction preconditions did not match current values.ContentionHistogramHistogramHow many contended writes were encountered |
| RequestType: | Read                                                         |
| Description: | Metrics related to standard read requests.                   |
| Metrics:     | NameTypeDescriptionTimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of read failures encountered. LatencyRead latency.UnavailablesCounterNumber of unavailable exceptions encountered. |
| RequestType: | RangeSlice                                                   |
| Description: | Metrics related to token range read requests.                |
| Metrics:     | NameTypeDescriptionTimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of range query failures encountered. LatencyRange query latency.UnavailablesCounterNumber of unavailable exceptions encountered. |
| RequestType: | Write                                                        |
| Description: | Metrics related to regular write requests.                   |
| Metrics:     | NameTypeDescriptionTimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of write failures encountered. LatencyWrite latency.UnavailablesCounterNumber of unavailable exceptions encountered. |
| RequestType: | ViewWrite                                                    |
| Description: | Metrics related to materialized view write wrtes.            |
| Metrics:     | TimeoutsCounterNumber of timeouts encountered.FailuresCounterNumber of transaction failures encountered.UnavailablesCounterNumber of unavailable exceptions encountered.ViewReplicasAttemptedCounterTotal number of attempted view replica writes.ViewReplicasSuccessCounterTotal number of succeded view replica writes.ViewPendingMutationsGauge<Long>ViewReplicasAttempted - ViewReplicasSuccess.ViewWriteLatencyTimerTime between when mutation is applied to base table and when CL.ONE is achieved on view. |

## Cache Metrics

Cassandra caches have metrics to track the effectivness of the caches. Though the `Table Metrics` might be more useful.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Cache.<MetricName>.<CacheName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Cache scope=<CacheName> name=<MetricName>`

| Name                      | Type           | Description                             |
| ------------------------- | -------------- | --------------------------------------- |
| Capacity                  | Gauge<Long>    | Cache capacity in bytes.                |
| Entries                   | Gauge<Integer> | Total number of cache entries.          |
| FifteenMinuteCacheHitRate | Gauge<Double>  | 15m cache hit rate.                     |
| FiveMinuteCacheHitRate    | Gauge<Double>  | 5m cache hit rate.                      |
| OneMinuteCacheHitRate     | Gauge<Double>  | 1m cache hit rate.                      |
| HitRate                   | Gauge<Double>  | All time cache hit rate.                |
| Hits                      | Meter          | Total number of cache hits.             |
| Misses                    | Meter          | Total number of cache misses.           |
| MissLatency               | Timer          | Latency of misses.                      |
| Requests                  | Gauge<Long>    | Total number of cache requests.         |
| Size                      | Gauge<Long>    | Total size of occupied cache, in bytes. |

The following caches are covered:

| Name         | Description                                   |
| ------------ | --------------------------------------------- |
| CounterCache | Keeps hot counters in memory for performance. |
| ChunkCache   | In process uncompressed page cache.           |
| KeyCache     | Cache for partition to sstable offsets.       |
| RowCache     | Cache for rows kept in memory.                |

Note

Misses and MissLatency are only defined for the ChunkCache

## CQL Metrics

Metrics specific to CQL prepared statement caching.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.CQL.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=CQL name=<MetricName>`

| Name                       | Type           | Description                                                  |
| -------------------------- | -------------- | ------------------------------------------------------------ |
| PreparedStatementsCount    | Gauge<Integer> | Number of cached prepared statements.                        |
| PreparedStatementsEvicted  | Counter        | Number of prepared statements evicted from the prepared statement cache |
| PreparedStatementsExecuted | Counter        | Number of prepared statements executed.                      |
| RegularStatementsExecuted  | Counter        | Number of **non** prepared statements executed.              |
| PreparedStatementsRatio    | Gauge<Double>  | Percentage of statements that are prepared vs unprepared.    |

## DroppedMessage Metrics

Metrics specific to tracking dropped messages for different types of requests. Dropped writes are stored and retried by `Hinted Handoff`

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.DroppedMessages.<MetricName>.<Type>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=DroppedMetrics scope=<Type> name=<MetricName>`

| Name                    | Type  | Description                       |
| ----------------------- | ----- | --------------------------------- |
| CrossNodeDroppedLatency | Timer | The dropped latency across nodes. |
| InternalDroppedLatency  | Timer | The dropped latency within node.  |
| Dropped                 | Meter | Number of dropped messages.       |

The different types of messages tracked are:

| Name             | Description                                  |
| ---------------- | -------------------------------------------- |
| BATCH_STORE      | Batchlog write                               |
| BATCH_REMOVE     | Batchlog cleanup (after succesfully applied) |
| COUNTER_MUTATION | Counter writes                               |
| HINT             | Hint replay                                  |
| MUTATION         | Regular writes                               |
| READ             | Regular reads                                |
| READ_REPAIR      | Read repair                                  |
| PAGED_SLICE      | Paged read                                   |
| RANGE_SLICE      | Token range read                             |
| REQUEST_RESPONSE | RPC Callbacks                                |
| _TRACE           | Tracing writes                               |

## Streaming Metrics

Metrics reported during `Streaming` operations, such as repair, bootstrap, rebuild.

These metrics are specific to a peer endpoint, with the source node being the node you are pulling the metrics from.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Streaming.<MetricName>.<PeerIP>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Streaming scope=<PeerIP> name=<MetricName>`

| Name          | Type    | Description                                                  |
| ------------- | ------- | ------------------------------------------------------------ |
| IncomingBytes | Counter | Number of bytes streamed to this node from the peer.         |
| OutgoingBytes | Counter | Number of bytes streamed to the peer endpoint from this node. |

## Compaction Metrics

> https://docs.datastax.com/en/cassandra-oss/3.x/cassandra/operations/opsCompactionMetrics.html

Monitoring compaction performance is an important aspect of knowing when to add capacity to your cluster. The following attributes are exposed through CompactionManagerMBean:

| Attribute                 | Description                                            |
| ------------------------- | ------------------------------------------------------ |
| BytesCompacted            | Total number of bytes compacted since server [re]start |
| CompletedTasks            | Number of completed compactions since server [re]start |
| PendingTasks              | Estimated number of compactions remaining to perform   |
| TotalCompactionsCompleted | Total number of compactions since server [re]start     |



Metrics specific to `Compaction` work.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Compaction.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Compaction name=<MetricName>`

| Name                      | Type                                     | Description                                                  |
| ------------------------- | ---------------------------------------- | ------------------------------------------------------------ |
| BytesCompacted            | Counter                                  | Total number of bytes compacted since server [re]start.      |
| PendingTasks              | Gauge<Integer>                           | Estimated number of compactions remaining to perform.        |
| CompletedTasks            | Gauge<Long>                              | Number of completed compactions since server [re]start.      |
| TotalCompactionsCompleted | Meter                                    | Throughput of completed compactions since server [re]start.  |
| PendingTasksByTableName   | Gauge<Map<String, Map<String, Integer>>> | Estimated number of compactions remaining to perform, grouped by keyspace and then table name. This info is also kept in `Table Metrics`. |

## CommitLog Metrics

Metrics specific to the `CommitLog`

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.CommitLog.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=CommitLog name=<MetricName>`

| Name                       | Type        | Description                                                  |
| -------------------------- | ----------- | ------------------------------------------------------------ |
| CompletedTasks             | Gauge<Long> | Total number of commit log messages written since [re]start. |
| PendingTasks               | Gauge<Long> | Number of commit log messages written but yet to be fsync’d. |
| TotalCommitLogSize         | Gauge<Long> | Current size, in bytes, used by all the commit log segments. |
| WaitingOnSegmentAllocation | Timer       | Time spent waiting for a CommitLogSegment to be allocated - under normal conditions this should be zero. |
| WaitingOnCommit            | Timer       | The time spent waiting on CL fsync; for Periodic this is only occurs when the sync is lagging its sync interval. |

## Storage Metrics

Metrics specific to the storage engine.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Storage.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Storage name=<MetricName>`

| Name                 | Type    | Description                                                  |
| -------------------- | ------- | ------------------------------------------------------------ |
| Exceptions           | Counter | Number of internal exceptions caught. Under normal exceptions this should be zero. |
| Load                 | Counter | Size, in bytes, of the on disk data size this node manages.  |
| TotalHints           | Counter | Number of hint messages written to this node since [re]start. Includes one entry for each host to be hinted per hint. |
| TotalHintsInProgress | Counter | Number of hints attemping to be sent currently.              |

## HintedHandoff Metrics

Metrics specific to Hinted Handoff. There are also some metrics related to hints tracked in `Storage Metrics`

These metrics include the peer endpoint **in the metric name**

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.HintedHandOffManager.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=HintedHandOffManager name=<MetricName>`

| Name                      | Type    | Description                                                  |
| ------------------------- | ------- | ------------------------------------------------------------ |
| Hints_created-<PeerIP>    | Counter | Number of hints on disk for this peer.                       |
| Hints_not_stored-<PeerIP> | Counter | Number of hints not stored for this peer, due to being down past the configured hint window. |

## SSTable Index Metrics

Metrics specific to the SSTable index metadata.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Index.<MetricName>.RowIndexEntry`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Index scope=RowIndexEntry name=<MetricName>`

| Name             | Type      | Description                                                  |
| ---------------- | --------- | ------------------------------------------------------------ |
| IndexedEntrySize | Histogram | Histogram of the on-heap size, in bytes, of the index across all SSTables. |
| IndexInfoCount   | Histogram | Histogram of the number of on-heap index entries managed across all SSTables. |
| IndexInfoGets    | Histogram | Histogram of the number index seeks performed per SSTable.   |

## BufferPool Metrics

Metrics specific to the internal recycled buffer pool Cassandra manages. This pool is meant to keep allocations and GC lower by recycling on and off heap buffers.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.BufferPool.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=BufferPool name=<MetricName>`

| Name   | Type        | Description                                                  |
| ------ | ----------- | ------------------------------------------------------------ |
| Size   | Gauge<Long> | Size, in bytes, of the managed buffer pool                   |
| Misses | Meter       | The rate of misses in the pool. The higher this is the more allocations incurred. |

## Client Metrics

Metrics specifc to client managment.

Reported name format:

- **Metric Name**

  `org.apache.cassandra.metrics.Client.<MetricName>`

- **JMX MBean**

  `org.apache.cassandra.metrics:type=Client name=<MetricName>`

| Name                   | Type    | Description                                                  |
| ---------------------- | ------- | ------------------------------------------------------------ |
| connectedNativeClients | Counter | Number of clients connected to this nodes native protocol server |
| connectedThriftClients | Counter | Number of clients connected to this nodes thrift protocol server |

## JVM Metrics

JVM metrics such as memory and garbage collection statistics can either be accessed by connecting to the JVM using JMX or can be exported using [Metric Reporters](https://murukeshm.github.io/cassandra/3.10/operating/metrics.html#metric-reporters).

### BufferPool

- **Metric Name**

  `jvm.buffers.<direct|mapped>.<MetricName>`

- **JMX MBean**

  `java.nio:type=BufferPool name=<direct|mapped>`

| Name     | Type        | Description                                                  |
| -------- | ----------- | ------------------------------------------------------------ |
| Capacity | Gauge<Long> | Estimated total capacity of the buffers in this pool         |
| Count    | Gauge<Long> | Estimated number of buffers in the pool                      |
| Used     | Gauge<Long> | Estimated memory that the Java virtual machine is using for this buffer pool |

### FileDescriptorRatio

- **Metric Name**

  `jvm.fd.<MetricName>`

- **JMX MBean**

  `java.lang:type=OperatingSystem name=<OpenFileDescriptorCount|MaxFileDescriptorCount>`

| Name  | Type  | Description                             |
| ----- | ----- | --------------------------------------- |
| Usage | Ratio | Ratio of used to total file descriptors |

### GarbageCollector

- **Metric Name**

  `jvm.gc.<gc_type>.<MetricName>`

- **JMX MBean**

  `java.lang:type=GarbageCollector name=<gc_type>`

| Name  | Type        | Description                                                  |
| ----- | ----------- | ------------------------------------------------------------ |
| Count | Gauge<Long> | Total number of collections that have occurred               |
| Time  | Gauge<Long> | Approximate accumulated collection elapsed time in milliseconds |

### Memory

- **Metric Name**

  `jvm.memory.<heap/non-heap/total>.<MetricName>`

- **JMX MBean**

  `java.lang:type=Memory`

| Committed | Gauge<Long> | Amount of memory in bytes that is committed for the JVM to use |
| --------- | ----------- | ------------------------------------------------------------ |
| Init      | Gauge<Long> | Amount of memory in bytes that the JVM initially requests from the OS |
| Max       | Gauge<Long> | Maximum amount of memory in bytes that can be used for memory management |
| Usage     | Ratio       | Ratio of used to maximum memory                              |
| Used      | Gauge<Long> | Amount of used memory in bytes                               |

### MemoryPool

- **Metric Name**

  `jvm.memory.pools.<memory_pool>.<MetricName>`

- **JMX MBean**

  `java.lang:type=MemoryPool name=<memory_pool>`

| Committed | Gauge<Long> | Amount of memory in bytes that is committed for the JVM to use |
| --------- | ----------- | ------------------------------------------------------------ |
| Init      | Gauge<Long> | Amount of memory in bytes that the JVM initially requests from the OS |
| Max       | Gauge<Long> | Maximum amount of memory in bytes that can be used for memory management |
| Usage     | Ratio       | Ratio of used to maximum memory                              |
| Used      | Gauge<Long> | Amount of used memory in bytes                               |
