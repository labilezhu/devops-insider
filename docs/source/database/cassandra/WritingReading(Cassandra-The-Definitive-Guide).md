> Jeff Carpenter, Eben Hewitt - Cassandra The Definitive Guide, Third Edition Distributed Data at Web Scale - O'Reilly Media (2022)

# Writing
## Write Consistency Levels

![image-20221017155321401](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017155321401.png)

## The Cassandra Write Path

The *write path* describes how data modification queries initiated by clients are processed, eventually resulting in the data being stored on disk. We’ll examine the write path in terms of both interactions between nodes and the internal process of storing data on an individual node. An overview of the write path interactions between nodes in a multiple data center cluster is shown in Figure 9-1.

The write path begins when a client initiates a write query to a Cassandra node that serves as the coordinator for this request. The coordinator node uses the partitioner to identify which nodes in the cluster are replicas, according to the replication factor for the keyspace. The coordinator node may itself be a replica, especially if the client is using a token-aware load balancing policy. If the coordinator knows that there are not enough replicas up to satisfy the requested consistency level, it returns an error immediately.

Next, the coordinator node sends simultaneous write requests to all local replicas for the data being written. If the cluster spans multiple data centers, the *local coordinator* node selects a *remote coordinator* in each of the other data centers to forward the write to the replicas in that data center. Each of the remote replicas acknowledges the write directly to the original coordinator node.

This ensures that all nodes will get the write as long as they are up. Nodes that are down will not have consistent data, but they will be repaired via one of the anti-entropy mechanisms: `hinted handoff`, read repair, or anti-entropy repair.

The coordinator waits for the replicas to respond. Once a sufficient number of replicas have responded to satisfy the consistency level, the coordinator acknowledges the write to the client. If a replica doesn’t respond within the timeout, it is presumed to be down, and a hint is stored for the write. A hint does not count as a successful replica write unless the consistency level `ANY` is used.

![image-20221017155416433](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017155416433.png)

Figure 9-1. Interactions between nodes on the write path

Figure 9-2 depicts the interactions that take place within each replica node to process a write request. These steps are common in databases that share the log-structured merge tree design we explored in Chapter 6.

![image-20221017155631835](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017155631835.png)

First, the replica node receives the write request and immediately writes the data to the commit log. Next, the replica node writes the data to a memtable. If row caching is used and the row is in the cache, the row is invalidated. We’ll discuss caching in more detail under the read path.

If the write causes either the commit log or memtable to pass its maximum thresholds, a flush is scheduled to run. We’ll learn how to tune these thresholds in Chapter 13.

At this point, the write is considered to have succeeded and the node can reply to the coordinator node or client.

After returning, the node executes a flush if one was scheduled. The contents of each memtable are stored as SSTables on disk, and the commit log is cleared. After the flush completes, additional tasks are scheduled to check if compaction is needed, and then a compaction is performed if necessary.

### Commit log files

Cassandra writes commit logs to the filesystem as binary files. By default, the commit log files are found under the *$CASSANDRA_HOME/data/commitlog* directory.

Commit log files are named according to the pattern `CommitLog-<version><timestamp>`.log. For example: *`CommitLog-7-1566780133999.log`*. The *version* is an integer representing the commit log format. For example, the version for the 4.0 release is 7. You can find the versions in use by release in the `org.apache.cassandra.db.commitlog.CommitLogDescriptor` class.

### SSTable files

When SSTables are written to the filesystem during a flush, there are actually several files that are written per SSTable. Let’s take a look at the default location under the *$CASSANDRA_HOME/data/data* directory to see how the files are organized on disk.

> **Forcing SSTables to Disk**
>
> If you’re following along with the exercises in this book on a real Cassandra node, you may want to execute the `nodetool flush` command at this point, as you may not have entered enough data yet for Cassandra to have flushed data to disk automatically. You’ll learn more about this command in Chapter 12.

Looking in the *data* directory, you’ll see a directory for each keyspace. These directories, in turn, contain a directory for each table, consisting of the table name plus a UUID. The purpose of the UUID is to distinguish between multiple schema versions.

Each of these directories contains SSTable files that contain the stored data. Here is an example directory path: *hotel/hotels-3677bbb0155811e5899aa9fac1d00bce*.

Each SSTable is represented by multiple files that share a common naming scheme. The files are named according to the pattern `<version>-<generation>-<implementation>-<component>.db`. The significance of the pattern is as follows:

- version

  A two-character sequence representing the major/minor version of the SSTable format. For example, the version for the 4.0 release is *na*. You can learn more the about various versions in the `org.apache.cassandra.io.sstable.Descriptor` class

- generation

  An index number that is incremented every time a new SSTable is created for a table

- implementation

  A reference to the implementation of the `org.apache.cassandra.io.sstable.format.SSTableWriter` interface in use. As of the 4.0 release the value is “big,” which references the “Bigtable format” found in the `org.apache.cassandra.io.sstable.format.big.BigFormat` class

Each SSTable is broken up into multiple files or *components*. These are the components as of the 3.0 release:

- *Data.db*

  These are the files that store the actual data and are the only files that are preserved by Cassandra’s backup mechanisms, which you’ll learn about in Chapter 12.

- *CompressionInfo.db*

  Provides metadata about the compression of the *Data.db* file.

- *Digest.crc32*

  Contains a CRC32 checksum for the **-Data.db* file.

- *Filter.db*

  Contains the Bloom filter for this SSTable.

- *Index.db*

  Provides row and column offsets within the corresponding **-Data.db* file. The contents of this file are read into memory so that Cassandra knows exactly where to look when reading datafiles.

- *Summary.db*

  A sample of the index for even faster reads.

- *Statistics.db*

  Stores statistics about the SSTable that are used by the `nodetool tablehistograms` command.

- *TOC.txt*

  Lists the file components for this SSTable.

Older releases support different versions and filenames. Releases prior to 2.2 prepend the keyspace and table name to each file, while 2.2 and later releases leave these out because they can be inferred from the directory name.

We’ll investigate some tools for working with SSTable files in Chapter 12.

# Reading

## Read Consistency Levels

![image-20221017160440159](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017160440159.png)

## The Cassandra Read Path

Now let’s take a look at what happens when a client requests data. This is known as the *read path*. We’ll describe the read path from the perspective of a query for a single partition key, starting with the interactions between nodes shown in Figure 9-3.

![image-20221017160529058](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017160529058.png)

Figure 9-3. Interactions between nodes on the read path

The read path begins when a client initiates a read query to the coordinator node. As on the write path, the coordinator uses the partitioner to determine the replicas, and checks that there are enough replicas up to satisfy the requested consistency level. Another similarity to the write path is that a remote coordinator is selected per data center for any read queries that involve multiple data centers.

If the coordinator is not itself a replica, the coordinator then sends a read request to the fastest replica, as determined by the dynamic `snitch`. The coordinator node also sends a *`digest request`* to the other replicas. A digest request is similar to a standard read request, **except the replicas return a digest, or hash, of the requested data**.

The coordinator calculates the digest hash for data returned from the fastest replica and compares it to the digests returned from the other replicas. If the digests are consistent, and the desired consistency level has been met, then the data from the fastest replica can be returned. If the digests are not consistent, then the coordinator must perform a read repair, as discussed in the following section.

Figure 9-4 shows the interactions that take place within each replica node to process read requests.

![image-20221017160746807](WritingReading(Cassandra-The-Definitive-Guide).assets/image-20221017160746807.png)

When the replica node receives the read request, it first checks the row cache. If the row cache contains the data, it can be returned immediately. The row cache helps speed read performance for rows that are accessed frequently. We’ll discuss the pros and cons of row caching in Chapter 13.

If the data is not in the row cache, the replica node searches for the data in memtables and SSTables. There is only a single memtable for a given table, so that part of the search is straightforward. However, there are potentially many physical SSTables for a single Cassandra table, each of which may contain a portion of the requested data.

Cassandra implements several features to optimize the SSTable search: key caching, Bloom filters, SSTable indexes, and summary indexes.

The first step in searching SSTables on disk is to use a Bloom filter to determine whether the requested partition does not exist in a given SSTable, which would make it unnecessary to search that SSTable.

If the SSTable passes the Bloom filter, Cassandra checks the key cache to see if it contains the offset of the partition key in the SSTable. The key cache is implemented as a map structure in which the keys are a combination of the SSTable file descriptor and partition key, and the values are offset locations into SSTable files. The key cache helps to eliminate seeks within SSTable files for frequently accessed data, because the data can be read directly.

If the offset is not obtained from the key cache, Cassandra uses a two-level index stored on disk in order to locate the offset. The first-level index is the *`partition summary`*, which is used to obtain an offset for searching for the partition key within the second-level index, the *`partition index`*. The `partition index` is where the offset into the SSTable for the partition key is stored.

If the offset for the partition key is found, Cassandra accesses the SSTable at the specified offset and starts reading data. In the 3.6 release, a chunk cache was added to store chunks of data from SSTables that are accessed frequently; you’ll learn more about this in Chapter 13.

**Once data has been obtained from all of the SSTables, Cassandra merges the SSTable data and memtable data by selecting the value with the latest timestamp for each requested column.**

Finally, the merged data can be added to the row cache (if enabled) and returned to the client or coordinator node. A digest request is handled in much the same way as a regular read request, with the additional step that a digest is calculated on the result data and returned instead of the data itself.

## Read Repair

//TODO

## Deleting

//TODO

