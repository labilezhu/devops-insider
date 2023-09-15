# Recommended production settings

> https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configRecommendedSettings.html





DataStax recommends the following settings for using DataStax Enterprise (DSE) in production environments.

**CAUTION:** Depending on your environment, some of the following settings might not persist after reboot. Check with your system administrator to ensure these settings are viable for your environment.

Use the [Preflight check tool](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/tools/dsePreFlight.html#dsePreFlight) to run a collection of tests on a DSE node to detect and fix node configurations. The tool can detect and optionally fix many invalid or suboptimal configuration settings, such as user resource limits, swap, and disk settings.

## Configure the chunk cache

Beginning in DataStax Enterprise (DSE) 6.0, the amount of native memory used by the DSE process has increased significantly.

The main reason for this increase is the chunk cache (or file cache), which is like an OS page cache. The following sections provide additional information:

- See [Chunk cache history](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configRecommendedSettings.html#configFileCacheSettings__chunkCacheHistory) for a historical description of the chunk cache, and how it is calculated in DSE 6.0 and later.
- See [Chunk cache differences from OS page cache](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configRecommendedSettings.html#configFileCacheSettings__chunkCacheDifferences) to understand key differences between the chunk cache and the OS page cache.

Consider the following recommendations depending on workload type for your cluster.

DSE recommendations

Regarding DSE, consider the following recommendations when choosing the max direct memory and file cache size:

- Total server memory size
- Adequate memory for the OS and other applications
- Adequate memory for the Java heap size
- Adequate memory for native raw memory (such as bloom filters and off-heap memtables)

For 64 GB servers, the default settings are typically adequate. For larger servers, [increase the max direct memory](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/operations/opsConHeapSize.html#opsConHeapSize__jvmIncreaseMaxDirectMemory) (`-XX:MaxDirectMemorySize`), but leave approximately 15-20% of memory for the OS and other in-memory structures. The file cache size will be set automatically to half of that. This setting is acceptable, but the size could be increased gradually if the cache hit rate is too low and there is still available memory on the server.

DSE Search recommendations

Disabling asynchronous I/O (AIO) and explicitly setting the chunk cache size ([file_cache_size_in_mb](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configCassandra_yaml.html#configCassandra_yaml__file_cache_size_in_mb)) improves performance for most DSE Search workloads. When enforced, SSTables and Lucene segments, as well as other minor off-heap elements, will reside in the OS page cache and be managed by the kernel.

A potentially negative impact of disabling AIO might be measurably higher read latency when DSE goes to disk, in cases where the dataset is larger than available memory.

To disable AIO and set the chunk cache size, see [Disable AIO](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/search/tuningIndexing.html#tuningIndexing__aioTuning).

DSE Analytics recommendations

DSE Analytics relies heavily on memory for performance. Because Apache Spark™ effectively manages its own memory through the Apache Spark application settings, you must determine how much memory the Apache Spark application receives. Therefore, you must think about how much memory to allocate to the chunk cache versus how much memory to allocate for Apache Spark applications. Similar to DSE Search, you can disable AIO and lower the chunk cache size to provide Apache Spark with more memory.

DSE Graph recommendations

Because DSE Graph heavily relies on several different workloads, it’s important to follow the previous recommendations for the specific workload. If you use DSE Search or DSE Analytics with DSE Graph, lower the chunk cache and disable AIO for the best performance. If you use DSE Graph only on top of Apache Cassandra, increase the chunk cache gradually, leaving 15-20% of memory available for other processes.

Chunk cache differences from OS page cache

There are several differences between the chunk cache and the OS page cache, and a full description is outside the scope of this information. However, the following differences are relevant to DSE:

- Because the OS page cache is sized dynamically by the operating system, it can grow and shrink depending on the available server memory. The chunk cache must be sized statically.

  If the chunk cache is too small, the available server memory will be unused. For servers with large amounts of memory (50 GB or more), the memory is wasted. If the chunk cache is too large, the available memory on the server can reduce enough that the OS will kill the DSE process to avoid an out of memory issue.

  **Note:** At the time of writing, the size of the chunk cache cannot be changed dynamically so to change the size of the chunk cache the DSE process must be restarted.

- Restarting the DSE process will destroy the chunk cache, so each time the process is restarted, the chunk cache will be cold. The OS page cache only becomes cold after a server restart.

- The memory used by the file cache is part of the DSE process memory, and is therefore seen by the OS as user memory. However, the OS page cache memory is seen as buffer memory.

- The chunk cache uses mostly NIO direct memory, storing file chunks into NIO byte buffers. However, NIO does have an on-heap footprint, which DataStax is working to reduce.

Chunk cache history

The chunk cache is not new to Apache Cassandra, and was originally intended to cache small parts (chunks) of SSTable files to make read operations faster. However, the default file access mode was memory mapped until DSE 5.1, so the chunk cache had a secondary role and its size was limited to 512 MB.

**Note:** The default setting of 512 MB was configured by the `file_cache_size_in_mb` parameter in cassandra.yaml.

In DSE 6.0 and later, the [chunk cache](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configCassandra_yaml.html#configCassandra_yaml__file_cache_size_in_mb) has increased relevance, not just because it replaces the OS page cache for database read operations, but because it is a central component of the asynchronous thread-per-core (TPC) architecture.

By default, the chunk cache is configured to use the following portion of the max direct memory:

- One-half (½) of the max direct memory for the DSE process
- One-fourth (¼) of the max direct memory for tools

The max direct memory is calculated as one-half (½) of the system memory minus the JVM heap size:

```
Max direct memory = ((system memory - JVM heap size))/2
```

You can explicitly configure the max direct memory by setting the JVM MaxDirectMemorySize (`-XX:MaxDirectMemorySize`) parameter. See [increasing the max direct memory](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/operations/opsConHeapSize.html#opsConHeapSize__jvmIncreaseMaxDirectMemory). Alternatively, you can override the max direct memory setting by explicitly configuring the [file_cache_size_in_mb](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configCassandra_yaml.html#configCassandra_yaml__file_cache_size_in_mb) parameter in [cassandra.yaml](javascript:;).

## Install the latest Java Virtual Machine

Configure your operating system to use the latest build of a [Technology Compatibility Kit](https://openjdk.java.net/groups/conformance/JckAccess/) (TCK) Certified OpenJDK version 8. For example, [OpenJDK 8](http://openjdk.java.net/) (1.8.0_151 minimum). Java 9 is not supported.

**Tip:** Although Oracle JRE/JDK 8 is supported, DataStax does more extensive testing on OpenJDK 8. This change is due to the end of public updates for Oracle JRE/JDK 8.

See the installation instructions for your operating system:

- [Installing Open JDK 8 on Debian or Ubuntu Systems](https://docs.datastax.com/en/jdk-install/doc/jdk-install/installOpenJdkDeb.html)
- [Installing OpenJDK 8 on RHEL-based Systems](https://docs.datastax.com/en/jdk-install/doc/jdk-install/installOpenJdkRHEL.html)

## Synchronize clocks

Use Network Time Protocol (NTP) to synchronize the clocks on all nodes and application servers.

Synchronizing clocks is required because DataStax Enterprise (DSE) overwrites a column only if there is another version whose timestamp is more recent, which can happen when machines are in different locations.

DSE timestamps are encoded as microseconds because UNIX Epoch time does not include timezone information. The timestamp for all writes in DSE is Universal Time Coordinated (UTC). DataStax recommends converting to local time only when generating output to be read by humans.

1. Install NTP for your operating system:

   | Operating system                                             | Command                        |
   | ------------------------------------------------------------ | ------------------------------ |
   | Debian-based system                                          | `sudo apt-get install ntpdate` |
   | RHEL-based system1                                           | `sudo yum install ntpdate `    |
   | 1On RHEL 7 and later, [chrony](https://chrony.tuxfamily.org/index.html) is the default network time protocol daemon. The configuration file for chrony is located in /etc/chrony.conf on these systems. |                                |

2. Start the NTP service on all nodes:

   ```bash
   sudo service ntp start -x
   ```

3. Run the

    

   ntupdate

    

   command to synchronize clocks:

   ```bash
   sudo ntpdate 1.ro.pool.ntp.org
   ```

4. Verify that your NTP configuration is working:

   ```bash
   ntpstat
   ```

## Set kernel parameters

Configure the following kernel parameters for optimal traffic and user limits.

Run the following command to view all current Linux kernel settings:

```bash
sudo sysctl -a
```

### TCP settings

During low traffic intervals, a firewall configured with an idle connection timeout can close connections to local nodes and nodes in other data centers. To prevent connections between nodes from timing out, set the following network kernel settings:

1. Set the following TCP keepalive timeout values:

   ```bash
   sudo sysctl -w \
   net.ipv4.tcp_keepalive_time=60 \
   net.ipv4.tcp_keepalive_probes=3 \
   net.ipv4.tcp_keepalive_intvl=10
   ```

   These values set the TCP keepalive timeout to 60 seconds with 3 probes, 10 seconds gap between each. The settings detect dead TCP connections after 90 seconds (60 + 10 + 10 + 10). The additional traffic is negligible, and permanently leaving these settings is not an issue. See [Firewall idle connection timeout causes nodes to lose communication during low traffic times on Linux ](https://support.datastax.com/hc/en-us/articles/360004048917).

2. Change the following settings to handle thousands of concurrent connections used by the database:

   ```bash
   sudo sysctl -w \
   net.core.rmem_max=16777216 \
   net.core.wmem_max=16777216 \
   net.core.rmem_default=16777216 \
   net.core.wmem_default=16777216 \
   net.core.optmem_max=40960 \
   net.ipv4.tcp_rmem='4096 87380 16777216' \
   net.ipv4.tcp_wmem='4096 65536 16777216'
   ```

**Tip:** Instead of changing the system TCP settings, you can prevent reset connections during streaming by tuning the [streaming_keep_alive_period_in_secs](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configCassandra_yaml.html#configCassandra_yaml__streaming_keep_alive_period_in_secs) setting in [cassandra.yaml](javascript:;).

### Set user resource limits

Use the ulimit -a command to view the current limits. Although limits can also be temporarily set using this command, DataStax recommends making the changes permanent.

For more information, see [Recommended production settings](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/config/configRecommendedSettings.html#configRecommendedSettings).

Debian-based systems

1. Edit the

    

   /etc/pam.d/su

    

   file and uncomment the following line to enable the

    

   ```
   pam_limits.so
   ```

    

   module:

   ```
   session    required   pam_limits.so
   ```

   This change to the PAM configuration file ensures that the system reads the files in the /etc/security/limits.d directory.

2. If you run DSE as root, some Linux distributions (such as Ubuntu), require setting the limits for the root user explicitly instead of using

    

   cassandra_user

   :

   ```
   root - memlock unlimited
   root - nofile 1048576
   root - nproc 32768
   root - as unlimited
   ```

RHEL-based systems

1. Set the

    

   nproc

    

   limits to

    

   32768

    

   in the

    

   /etc/security/limits.d/90-nproc.conf

    

   configuration file:

   ```
   cassandra_user - nproc 32768
   ```

All systems

1. Add the following line to

    

   /etc/sysctl.conf

   :

   ```
   vm.max_map_count = 1048575
   ```

2. Open the configuration file for your installation type:

   | Installation type    | Configuration file                    |
   | -------------------- | ------------------------------------- |
   | Tarball installation | /etc/security/limits.conf             |
   | Package installation | /etc/security/limits.d/cassandra.conf |

3. Configure the following settings for the

    

   ```
   <cassandra_user>
   ```

    

   in the configuration file:

   ```
   <cassandra_user> - memlock unlimited
   <cassandra_user> - nofile 1048576
   <cassandra_user> - nproc 32768
   <cassandra_user> - as unlimited
   ```

4. Reboot the server or run the following command to make all changes take effect:

   ```bash
   sudo sysctl -p
   ```

### Persist updated settings

1. Add the following values to the

    

   /etc/sysctl.conf

    

   file:

   ```
   net.ipv4.tcp_keepalive_time=60
   net.ipv4.tcp_keepalive_probes=3
   net.ipv4.tcp_keepalive_intvl=10
   net.core.rmem_max=16777216
   net.core.wmem_max=16777216
   net.core.rmem_default=16777216
   net.core.wmem_default=16777216
   net.core.optmem_max=40960
   net.ipv4.tcp_rmem=4096 87380 16777216
   net.ipv4.tcp_wmem=4096 65536 16777216
   ```

2. Load the settings using one of the following commands:

   ```bash
   sudo sysctl -p /etc/sysctl.conf
   ```

   ```bash
   sudo sysctl -p /etc/sysctl.d/*.conf
   ```

3. To confirm the user limits are applied to the DSE process, run the following command where

    

   pid

    

   is the process ID of the currently running DSE process:

   ```bash
   cat /proc/pid/limits
   ```

## Disable settings that impact performance

Disable the following settings, which can cause issues with performance.

### Disable CPU frequency scaling

Recent Linux systems include a feature called CPU frequency scaling or CPU speed scaling. This feature allows a server's clock speed to be dynamically adjusted so that the server can run at lower clock speeds when the demand or load is low. This change reduces the server's power consumption and heat output, which significantly impacts cooling costs. Unfortunately, this behavior has a detrimental effect on servers running DSE, because throughput can be capped at a lower rate.

On most Linux systems, a `CPUfreq` governor manages the scaling of frequencies based on defined rules. The default `ondemand` governor switches the clock frequency to maximum when demand is high, and switches to the lowest frequency when the system is idle.

**Important:** Do not use governors that lower the CPU frequency. To ensure optimal performance, reconfigure all CPUs to use the `performance` governor, which locks the frequency at maximum.

The performance governor will not switch frequencies, which means that power savings will be bypassed to always run at maximum throughput. On most systems, run the following command to set the governor:

```
for CPUFREQ in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
do
    [ -f $CPUFREQ ] || continue
    echo -n performance > $CPUFREQ
done
```

**Tip:** If this directory does not exist on your system, refer to one of the following pages based on your operating system:

- Debian-based systems: [cpufreq-set command on Debian systems](http://manpages.ubuntu.com/manpages/bionic/man1/cpufreq-set.1.html)
- RHEL-based systems: [CPUfreq setup on RHEL systems](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/power_management_guide/cpufreq_setup#enabling_a_cpufreq_governor)

For more information, see [High server load and latency when CPU frequency scaling is enabled](https://support.datastax.com/hc/en-us/articles/115003018063) in the DataStax Help Center.

### Disable zone_reclaim_mode on NUMA systems

The Linux kernel can be inconsistent in enabling/disabling zone_reclaim_mode, which can result in odd performance problems.

To ensure that zone_reclaim_mode is disabled:

```bash
echo 0 > /proc/sys/vm/zone_reclaim_mode
```

For more information, see [Peculiar Linux kernel performance problem on NUMA systems](https://docs.datastax.com/en/dse-trblshoot/doc/troubleshooting/zoneReclaimMode.html).

### Disable swap

Failure to disable swap entirely can severely lower performance. Because the database has multiple replicas and transparent failover, it is preferable for a replica to be killed immediately when memory is low rather than go into swap. This allows traffic to be immediately redirected to a functioning replica instead of continuing to hit the replica that has high latency due to swapping. If your system has a lot of DRAM, swapping still lowers performance significantly because the OS swaps out executable code so that more DRAM is available for caching disks.

If you insist on using swap, you can set vm.swappiness=1. This allows the kernel swap out the absolute least used parts.

```bash
sudo swapoff --all
```

To make this change permanent, remove all swap file entries from /etc/fstab.

For more information, see [Nodes seem to freeze after some period of time](https://docs.datastax.com/en/dse-trblshoot/doc/troubleshooting/nodeFreeze.html).

## Optimize disk settings

The default disk configurations on most Linux distributions are not optimal. Follow these steps to optimize settings for your Solid State Drives (SSDs) or spinning disks.

**Note:** Complete the optimization settings for either SSDs or spinning disks. Do not complete both procedures for either storage type.

### Optimize SSDs

Complete the following steps to ensure the best settings for SSDs.

1. Ensure that the

    

   ```
   SysFS
   ```

    

   rotational flag is set to

    

   ```
   false
   ```

    

   (zero).

   This overrides any detection by the operating system to ensure the drive is considered an SSD.

2. Apply the same rotational flag setting for any block devices created from SSD storage, such as mdarrays.

3. Determine your devices by running

    

   ```
   lsblk
   ```

   :

   ```bash
   lsblk
   ```

   ```
   NAME   MAJ:MIN RM SIZE RO TYPE MOUNTPOINT
   vda    253:0    0  32G  0 disk
   |
   |-sda1 253:1    0   8M  0 part
   |-sda2 253:2    0  32G  0 part /
   ```

   In this example, the current devices are `sda1` and `sda2`.

4. Set the IO scheduler to either

    

   ```
   deadline
   ```

    

   or

    

   ```
   noop
   ```

    

   for each of the listed devices:

   For example:

   ```bash
   echo deadline > /sys/block/device_name/queue/scheduler
   ```

   where device_name is the name of the device you want to apply settings for.

   - The deadline scheduler optimizes requests to minimize IO latency. If in doubt, use the deadline scheduler.

     ```bash
     echo deadline > /sys/block/device_name/queue/scheduler
     ```

   - The

      

     ```
     noop
     ```

      

     scheduler is the right choice when the target block device is an array of SSDs behind a high-end IO controller that performs IO optimization.

     ```bash
     echo noop > /sys/block/device_name/queue/scheduler
     ```

5. Set the

    

   ```
   nr_requests
   ```

    

   value to indicate the maximum number of read and write requests that can be queued:

   | Machine size   | Value                                              |
   | -------------- | -------------------------------------------------- |
   | Large machines | `echo 128 sys/block/device_name/queue/nr_requests` |
   | Small machines | `echo 32 sys/block/device_name/queue/nr_requests`  |

6. Set the

    

   ```
   readahead
   ```

    

   value for the block device to 8 KB.

   This setting tells the operating system not to read extra bytes, which can increase IO time and pollute the cache with bytes that weren’t requested by the user.

   **Note:** The recommended `readahead` setting for RAID on SSDs is the same as that for SSDs that are not being used in a RAID installation.

   1. Open /etc/rc.local for editing.

   2. Add the following lines to set the

       

      ```
      readahead
      ```

       

      on startup:

      ```
      touch /var/lock/subsys/local
      echo 0 > /sys/class/block/sda/queue/rotational
      echo 8 > /sys/class/block/sda/queue/read_ahead_kb
      ```

   3. Save and close /etc/rc.local.

### Optimize spinning disks

1. Check to ensure read-ahead value is not set to 65536:

   ```bash
   sudo blockdev --report /dev/spinning_disk
   ```

2. Set the

    

   ```
   readahead
   ```

    

   to 128, which is the recommended value:

   ```bash
   sudo blockdev --setra 128 /dev/spinning_disk
   ```

## Set the heap size for Java garbage collection

The default JVM garbage collection (GC)is G1 for DSE 5.1 and later.

**Note:** DataStax does not recommend using G1 when using Java 7. This is due to a problem with class unloading in G1. In Java 7, PermGen fills up indefinitely until a full GC is performed.

Heap size is usually between ¼ and ½ of system memory. Do not devote all memory to heap because it is also used for offheap cache and file system cache.

See [Tuning Java Virtual Machine](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/operations/opsTuneJVM.html) for more information on tuning the Java Virtual Machine (JVM).

**Important:** If you want to use Concurrent-Mark-Sweep (CMS) garbage collection, contact the [DataStax Services team](https://www.datastax.com/products/services) for configuration help. [Tuning Java resources](https://docs.datastax.com/eol/en/dse/6.7/dse-admin/datastax_enterprise/operations/opsTuneJVM.html) provides details on circumstances where CMS is recommended, though using CMS requires time, expertise, and repeated testing to achieve optimal results.

The easiest way to determine the optimum heap size for your environment is:

1. Set the `MAX_HEAP_SIZE` in the [jvm.options](javascript:;) file to a high arbitrary value on a single node.
2. View the heap used by that node:
   - Enable GC logging and check the logs to see trends.
   - Use List view in OpsCenter.
3. Use the value for setting the heap size in the cluster.

**Note:** This method decreases performance for the test node, but generally does not significantly reduce cluster performance.

If you don't see improved performance, contact the [DataStax Services team](https://www.datastax.com/products/services) for additional help in tuning the JVM.

## Check Java Hugepages settings

Many modern Linux distributions ship with the Transparent Hugepages feature enabled by default. When Linux uses Transparent Hugepages, the kernel tries to allocate memory in large chunks (usually 2MB), rather than 4K. This allocation can improve performance by reducing the number of pages the CPU must track. However, some applications still allocate memory based on 4K pages, which can cause noticeable performance problems when Linux tries to defragment 2MB pages.

For more information, see the [Cassandra Java Huge Pages](https://tobert.github.io/tldr/cassandra-java-huge-pages.html) blog and this [RedHat bug report](https://bugzilla.redhat.com/show_bug.cgi?id=879801).

To solve this problem, disable defrag for Transparent Hugepages:

```bash
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag
```

For more information, including a temporary fix, see [No DSE processing but high CPU usage](https://docs.datastax.com/en/dse-trblshoot/doc/troubleshooting/highCPU.html).
