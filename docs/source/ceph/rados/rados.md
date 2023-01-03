# RADOS



> [https://docs.ceph.com/en/latest/rados/configuration/storage-devices/](https://docs.ceph.com/en/latest/rados/configuration/storage-devices/)



## STORAGE DEVICES

There are several Ceph daemons in a storage cluster:

- **Ceph OSDs** (Object Storage Daemons) store most of the data in Ceph. Usually each OSD is backed by a single storage device. This can be a traditional hard disk (HDD) or a solid state disk (SSD). OSDs can also be backed by a combination of devices: for example, a HDD for most data and an SSD (or partition of an SSD) for some metadata. The number of OSDs in a cluster is usually a function(函数关系) of the amount of data to be stored, the size of each storage device, and the level and type of redundancy specified (replication or erasure coding).
- **Ceph Monitor** daemons manage critical cluster state. This includes `cluster membership` and `authentication information`. Small clusters require only a few gigabytes of storage to hold the monitor database. In large clusters, however, the monitor database can reach sizes of tens of gigabytes to hundreds of gigabytes.
- **Ceph Manager** daemons run alongside monitor daemons, providing additional monitoring and providing interfaces to external monitoring and management systems.



## OSD Back Ends

There are two ways that OSDs manage the data they store. As of the Luminous 12.2.z release, the default (and recommended) back end is *BlueStore*. Prior to the Luminous release, the default (and only) back end was *Filestore*.



## Bluestore

BlueStore is a special-purpose storage back end designed specifically for managing data on disk for Ceph OSD workloads. BlueStore’s design is based on a decade of experience of supporting and managing Filestore OSDs.

Key BlueStore features include:

- Direct management of storage devices. BlueStore consumes raw block devices or partitions. This avoids intervening layers of abstraction (such as local file systems like XFS) that can limit performance or add complexity.
- Metadata management with `RocksDB`. RocksDB’s key/value database is embedded in order to manage internal metadata, including the mapping of` object names` to `block locations on disk`.
- Full data and metadata checksumming. By default, all data and metadata written to BlueStore is protected by one or more checksums. No data or metadata is read from disk or returned to the user without being verified.
- Inline compression. Data can be optionally compressed before being written to disk.
- Multi-device metadata tiering. BlueStore allows its internal journal (write-ahead log) to be written to a separate, high-speed device (like an SSD, NVMe, or NVDIMM) for increased performance. If a significant amount of faster storage is available, internal metadata can be stored on the faster device.
- Efficient copy-on-write. RBD and CephFS snapshots rely on a copy-on-write *clone* mechanism that is implemented efficiently in BlueStore. This results in efficient I/O both for regular snapshots and for erasure-coded pools (which rely on cloning to implement efficient two-phase commits).

For more information, see [BlueStore Config Reference](https://docs.ceph.com/en/latest/rados/configuration/bluestore-config-ref/) and [BlueStore Migration](https://docs.ceph.com/en/latest/rados/operations/bluestore-migration/).





## CONFIGURING CEPH

> [https://docs.ceph.com/en/latest/rados/configuration/ceph-conf/](https://docs.ceph.com/en/latest/rados/configuration/ceph-conf/)



### CONFIG SOURCES

Each Ceph daemon, process, and library will pull its configuration from several sources, listed below. Sources later in the list will override those earlier in the list when both are present.

- the compiled-in default value
- the monitor cluster’s centralized configuration database
- a configuration file stored on the local host
- environment variables
- command line arguments
- runtime overrides set by an administrator

One of the first things a Ceph process does on startup is parse the configuration options provided via the command line, environment, and local configuration file. The process will then contact the monitor cluster to retrieve configuration stored centrally for the entire cluster. Once a complete view of the configuration is available, the daemon or process startup will proceed.



### BOOTSTRAP OPTIONS

Some configuration options affect the process’s ability to contact the monitors, to authenticate, and to retrieve the cluster-stored configuration. For this reason, these options might need to be stored locally on the node, and set by means of a local configuration file. These options include the following:

- mon_host, type:`str`

  This is a list of IP addresses or hostnames that are separated by commas, whitespace, or semicolons. Hostnames are resolved via DNS. All A and AAAA records are included in the search list.

- mon_host_override, type:`str`

  This is the list of monitors that the Ceph process **initially** contacts when first establishing communication with the Ceph cluster. This overrides the known monitor list that is derived from MonMap updates sent to older Ceph instances (like librados cluster handles). This option is expected to be useful primarily for debugging.

  - [`mon_dns_srv_name`](https://docs.ceph.com/en/latest/rados/configuration/mon-lookup-dns/#confval-mon_dns_srv_name)

  - [`mon_data`](https://docs.ceph.com/en/latest/rados/configuration/mon-config-ref/#confval-mon_data), [`osd_data`](https://docs.ceph.com/en/latest/rados/configuration/osd-config-ref/#confval-osd_data), `mds_data`, [`mgr_data`](https://docs.ceph.com/en/latest/mgr/administrator/#confval-mgr_data), and similar options that define which local directory the daemon stores its data in.

  - [`keyring`](https://docs.ceph.com/en/latest/rados/configuration/auth-config-ref/#confval-keyring), [`keyfile`](https://docs.ceph.com/en/latest/rados/configuration/auth-config-ref/#confval-keyfile), and/or [`key`](https://docs.ceph.com/en/latest/rados/configuration/auth-config-ref/#confval-key), which can be used to specify the authentication credential to use to authenticate with the monitor. Note that in most cases the default keyring location is in the data directory specified above.

In most cases, the default values of these options are suitable. There is one exception to this: the [`mon_host`](https://docs.ceph.com/en/latest/rados/configuration/ceph-conf/#confval-mon_host) option that identifies the addresses of the cluster’s monitors. When DNS is used to identify monitors, a local Ceph configuration file can be avoided entirely.



### RUNTIME CHANGES

In most cases, Ceph allows you to make changes to the configuration of a daemon at runtime. This capability is quite useful for increasing/decreasing logging output, enabling/disabling debug settings, and even for runtime optimization.

Generally speaking, configuration options can be updated in the usual way via the `ceph config set` command. For example, do enable the debug log level on a specific OSD:

```
ceph config set osd.123 debug_ms 20
```

Note that if the same option is also customized in a local configuration file, the monitor setting will be ignored (it has a lower priority than the local config file).













