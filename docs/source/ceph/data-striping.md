# data-striping（数据分片与IO并发）

> https://docs.ceph.com/en/quincy/architecture/#data-striping





Storage devices have throughput limitations, which impact performance and scalability. So storage systems often support striping–storing sequential pieces of information across multiple storage devices–to increase throughput and performance. The most common form of data striping comes from RAID. The RAID type most similar to Ceph’s striping is RAID 0, or a ‘striped volume’. Ceph’s striping offers the throughput of RAID 0 striping, the reliability of n-way RAID mirroring and faster recovery.

Ceph provides three types of clients: Ceph Block Device, Ceph File System, and Ceph Object Storage. A Ceph Client converts its data from the representation format it provides to its users (a block device image, RESTful objects, CephFS filesystem directories) into objects for storage in the Ceph Storage Cluster.



> Tips: 
>
> The objects Ceph stores in the Ceph Storage Cluster are not striped. Ceph Object Storage, `Ceph Block Device`, and the Ceph File System stripe their data over multiple `Ceph Storage Cluster objects`. Ceph Clients that write directly to the Ceph Storage Cluster via `librados` must perform the striping (and parallel I/O) for themselves to obtain these benefits.

The simplest Ceph striping format involves a `stripe count of 1 object`. Ceph Clients write stripe units to a Ceph Storage Cluster object until the object is at its maximum capacity, and then create another object for additional stripes of data. The simplest form of striping may be sufficient for small block device images, S3 or Swift objects and CephFS files. However, this simple form doesn’t take maximum advantage of Ceph’s ability to distribute data across placement groups, and consequently doesn’t improve performance very much. The following diagram depicts the simplest form of striping:



![img](data-striping.assets/ditaa-609b2033fcdfa0a95b663189cc63db38953866a1.png)

If you anticipate large images sizes, large S3 or Swift objects (e.g., video), or large CephFS directories, you may see considerable read/write performance improvements by striping client data over multiple objects within an object set. Significant write performance occurs when the client writes the stripe units to their corresponding objects in parallel. Since objects get mapped to different placement groups and further mapped to different OSDs, each write occurs in parallel at the maximum write speed. A write to a single drive would be limited by the head movement (e.g. 6ms per seek) and bandwidth of that one device (e.g. 100MB/s). By spreading that write over multiple objects (which map to different placement groups and OSDs) Ceph can reduce the number of seeks per drive and combine the throughput of multiple drives to achieve much faster write (or read) speeds.



## object set

In the following diagram, client data gets striped across an object set (`object set 1` in the following diagram) consisting of 4 objects, where the first stripe unit is `stripe unit 0` in `object 0`, and the fourth stripe unit is `stripe unit 3` in `object 3`. 



After writing the fourth stripe, the client determines if the object set is full. If the object set is not full, the client begins writing a stripe to the first object again (`object 0` in the following diagram). 



If the object set is full, the client creates a new object set (`object set 2` in the following diagram), and begins writing to the first stripe (`stripe unit 16`) in the first object in the new object set (`object 4` in the diagram below).

![img](data-striping.assets/ditaa-96a6fc80dad17fb53f161987ed64f0779930ffe1.png)

Three important variables determine how Ceph stripes data:

- **Object Size:** Objects in the Ceph Storage Cluster have a maximum configurable size (e.g., 2MB, 4MB, etc.). The object size should be large enough to accommodate many stripe units, and should be a multiple of the stripe unit.
  - 每个子 object 的最大空间
- **Stripe Width:** Stripes have a configurable unit size (e.g., 64kb). The Ceph Client divides the data it will write to objects into equally sized stripe units, except for the last stripe unit. A stripe width, should be a fraction of the Object Size so that an object may contain many stripe units.
  - 分片最小单元
- **Stripe Count:** The Ceph Client writes a sequence of stripe units over a series of objects determined by the stripe count. The series of objects is called an object set. After the Ceph Client writes to the last object in the object set, it returns to the first object in the object set.
  - 就是上面的 Object Set 中 object 的数量。



1. Once the Ceph Client has striped data to stripe units and mapped the stripe units to objects, 
2. Ceph’s CRUSH algorithm maps the objects to placement groups, 
3. the placement groups to Ceph OSD Daemons before the objects are stored as files on a storage drive.



> Since a client writes to a single pool, all data striped into objects get mapped to placement groups in the same pool. So they use the same CRUSH map and the same access controls.
>
> 每个 ceph client 只能使用一个 `ceph pool`



## FILE STRIPING

> https://docs.ceph.com/en/quincy/dev/file-striping/#file-striping



### CEPH_FILE_LAYOUT

Ceph distributes (stripes) the data for a given file across a number of underlying objects. The way file data is mapped to those objects is defined by the `ceph_file_layout` structure. The data distribution is a modified RAID 0, where data is striped across a set of objects up to a (per-file) fixed size, at which point another set of objects holds the file’s data. The second set also holds no more than the fixed amount of data, and then another set is used, and so on.

Defining some terminology will go a long way toward explaining the way file data is laid out across Ceph objects.



- - file

    A collection of contiguous data, named from the perspective of the Ceph client (i.e., a file on a Linux system using Ceph storage). The data for a file is divided into fixed-size “stripe units,” which are stored in ceph “objects.”

- - stripe unit

    The size (in bytes) of a block of data used in the RAID 0 distribution of a file. All stripe units for a file have equal size. The last stripe unit is typically incomplete–i.e. it represents the data at the end of the file as well as unused “space” beyond it up to the end of the fixed stripe unit size.

- - stripe count

    The number of consecutive stripe units that constitute a RAID 0 “stripe” of file data.

- - stripe

    A contiguous range of file data, RAID 0 striped across “stripe count” objects in fixed-size “stripe unit” blocks.

- - object

    A collection of data maintained by Ceph storage. Objects are used to hold portions of Ceph client files.

- - object set

    A set of objects that together represent a contiguous portion of a file.



Three fields in the `ceph_file_layout` structure define this mapping:

```c
u32 fl_stripe_unit;
u32 fl_stripe_count;
u32 fl_object_size;
```

The role of the first two fields should be clear from the definitions above.

The third field is the maximum size (in bytes) of an object used to back file data. The `object size` is a multiple of the `stripe unit`.

A file’s data is blocked into stripe units, and consecutive stripe units are stored on objects in an object set. The number of objects in a set is the same as the stripe count. No object storing file data will exceed the file’s designated object size, so after some fixed number of complete stripes, a new object set is used to store subsequent file data.

Note that by default, Ceph uses a simple striping strategy in which `object_size` equals `stripe_unit` and `stripe_count` is 1. This simply puts one `stripe_unit` in each object.

Here’s a more complex example:

```bash
file size = 1 trillion = 1000000000000 bytes

fl_stripe_unit = 64KB = 65536 bytes
fl_stripe_count = 5 stripe units per stripe
fl_object_size = 64GB = 68719476736 bytes
```

This means:

```bash
file stripe size = 64KB * 5 = 320KB = 327680 bytes
each object holds 64GB / 64KB = 1048576 stripe units
file object set size = 64GB * 5 = 320GB = 343597383680 bytes
    (also 1048576 stripe units * 327680 bytes per stripe unit)
```

So the file’s 1 trillion bytes can be divided into complete object sets, then complete stripes, then complete stripe units, and finally a single incomplete stripe unit:

```bash
- 1 trillion bytes / 320GB per object set = 2 complete object sets
    (with 312805232640 bytes remaining)
- 312805232640 bytes / 320KB per stripe = 954605 complete stripes
    (with 266240 bytes remaining)
- 266240 bytes / 64KB per stripe unit = 4 complete stripe units
    (with 4096 bytes remaining)
- and the final incomplete stripe unit holds those 4096 bytes.
```

The ASCII art below attempts to capture this:

```bash
   _________   _________   _________   _________   _________
  /object  0\ /object  1\ /object  2\ /object  3\ /object  4\
  +=========+ +=========+ +=========+ +=========+ +=========+
  |  stripe | |  stripe | |  stripe | |  stripe | |  stripe |
o |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | stripe 0
b |     0   | |     1   | |     2   | |     3   | |     4   |
j |---------| |---------| |---------| |---------| |---------|
e |  stripe | |  stripe | |  stripe | |  stripe | |  stripe |
c |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | stripe 1
t |     5   | |     6   | |     7   | |     8   | |     9   |
  |---------| |---------| |---------| |---------| |---------|
s |     .   | |     .   | |     .   | |     .   | |     .   |
e       .           .           .           .           .
t |     .   | |     .   | |     .   | |     .   | |     .   |
  |---------| |---------| |---------| |---------| |---------|
0 |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
  |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 1048575
  | 5242875 | | 5242876 | | 5242877 | | 5242878 | | 5242879 |
  \=========/ \=========/ \=========/ \=========/ \=========/

   _________   _________   _________   _________   _________
  /object  5\ /object  6\ /object  7\ /object  8\ /object  9\
  +=========+ +=========+ +=========+ +=========+ +=========+
  |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
o |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 1048576
b | 5242880 | | 5242881 | | 5242882 | | 5242883 | | 5242884 |
j |---------| |---------| |---------| |---------| |---------|
e |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
c |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 1048577
t | 5242885 | | 5242886 | | 5242887 | | 5242888 | | 5242889 |
  |---------| |---------| |---------| |---------| |---------|
s |     .   | |     .   | |     .   | |     .   | |     .   |
e       .           .           .           .           .
t |     .   | |     .   | |     .   | |     .   | |     .   |
  |---------| |---------| |---------| |---------| |---------|
1 |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
  |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 2097151
  | 10485755| | 10485756| | 10485757| | 10485758| | 10485759|
  \=========/ \=========/ \=========/ \=========/ \=========/

   _________   _________   _________   _________   _________
  /object 10\ /object 11\ /object 12\ /object 13\ /object 14\
  +=========+ +=========+ +=========+ +=========+ +=========+
  |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
o |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 2097152
b | 10485760| | 10485761| | 10485762| | 10485763| | 10485764|
j |---------| |---------| |---------| |---------| |---------|
e |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
c |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 2097153
t | 10485765| | 10485766| | 10485767| | 10485768| | 10485769|
  |---------| |---------| |---------| |---------| |---------|
s |     .   | |     .   | |     .   | |     .   | |     .   |
e       .           .           .           .           .
t |     .   | |     .   | |     .   | |     .   | |     .   |
  |---------| |---------| |---------| |---------| |---------|
2 |  stripe | |  stripe | |  stripe | |  stripe | |  stripe | stripe
  |   unit  | |   unit  | |   unit  | |   unit  | |   unit  | 3051756
  | 15258780| | 15258781| | 15258782| | 15258783| | 15258784|
  |---------| |---------| |---------| |---------| |---------|
  |  stripe | |  stripe | |  stripe | |  stripe | | (partial| (partial
  |   unit  | |   unit  | |   unit  | |   unit  | |  stripe | stripe
  | 15258785| | 15258786| | 15258787| | 15258788| |  unit)  | 3051757)
  \=========/ \=========/ \=========/ \=========/ \=========/
```





