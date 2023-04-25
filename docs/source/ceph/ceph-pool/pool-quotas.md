## pool quotas

## Case 1



> https://ceph-users.ceph.narkive.com/h6GPo1lk/ceph-rbd-and-pool-quotas



>Hi guys,
>
>quick question in regards to ceph -> rbd -> quotas per pool. I'd like to
>set a quota with max_bytes of a pool so that I can limit the amount a
>ceph client can use, like so:
>
>ceph osd pool set-quota pool1 max_bytes $(( 1024 * 1024 * 100 * 5))
>
>This is all working, e.g., if data gets written into the rbd mapped
>device (e.g. /dev/rbd0 --> /mnt/rbd0) and the pool reaches its capacity
>(full), it sets the ceph cluster health to WARN and notifies that the
>pool is full and the write stopps. However, when the write operation on
>the client stops it also hangs after that, e.g. the process can't be
>killed and waits for the device to respond, e.g.
>
>```
>USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND
>root 9226 0.0 0.0 5836 744 ? D 12:50 0:00 \_
>dd if=/dev/zero of=/mnt/rbd3/file count=500 bs=1M
>```
>
>Increasing the quota of the pool helps recovering the cluster health but
>the write is stuck forever on the client (needs to be rebooted to get
>rid of the process).
>
>Ideas?
>
>Cheers,
>Thomas



> Hi Thomas,
>
> You need kernel 4.7 for that to work - it will properly re-submit the
> write after the quota is increased.
>
> Thanks,
>
> Ilya



## Case 2



> https://lists.ceph.io/hyperkitty/list/ceph-users@ceph.io/thread/RW2D6YTUTN2YIIE44KRLBWYE5OX5EG34/



> Hello everyone, I have single Ceph cluster with multiple pools for purposes of Kubernetes PVs and Virtualization (biggest pool, using snapshots here). Every client cluster has its own pool with max_bytes and max_objects quotas in place. When quotas are reached, Ceph halts all write I/O (which I suppose is OK).



I am trying to setup best possible monitoring to prevent reaching pool quotas, but I am really struggling to find correct metrics Ceph is driven by, because almost every command I try gives me different results (rados df, rbd du, ceph health detail) and I feel like quotas are not usable for this purpose. 



After some fiddling with test pool I have somehow got it into state, when there are some phantom objects in stats (maybe also written to OSDs) which cannot be listed, accessed or deleted, but are counted into pool quotas. 

-> ceph version ceph version 14.2.10 (9f0d3f5a3ce352651da4c2437689144fcbec0131) nautilus (stable) I 

have my test pool here (quotas 4k objects, 15 GiB data): 



-> ceph osd pool ls detail | grep mirektest pool 2 'mirektest' replicated size 3 min_size 2 crush_rule 0 object_hash rjenkins pg_num 32 pgp_num 32 autoscale_mode warn last_change 21360 lfor 0/10929/10927 flags hashpspool,selfmanaged_snaps max_bytes 16106127360 max_objects 4000 stripe_width 0 target_size_bytes 10737418240 application rbd



Cluster health says pool it getting full: -> ceph health detail HEALTH_WARN 1 pools nearfull POOL_NEAR_FULL 1 pools nearfull   pool 'mirektest' has 3209 objects (max 4000)   pool 'mirektest' has 13 GiB (max 15 GiB) But pool looks completely empty (no output, so I tried JSON output also): -> rbd -p mirektest --format json ls [] -> rbd -p mirektest --format json du {"images":[],"total_provisioned_size":0,"total_used_size":0} I am not using pool snapshots: -> rados -p mirektest lssnap 0 snaps But there are some RADOS object in it: -> rados -p mirektest df POOL_NAME  *USED**OBJECTS*CLONES *COPIES*MISSING_ON_PRIMARY UNFOUND DEGRADED  RD_OPS   RD WR_OPS   WR USED COMPR UNDER COMPR mirektest *192 KiB*  *3209*   0  *9627*         0    0    0 23998515 192 GiB 1221030 297 GiB    0 B     0 B Okay, lets try to list them: -> rados -p mirektest ls --all   rbd_directory   rbd_info   rbd_trash Thats weird, expected more than 3 objetcs. Let's check it's sizes and content: -> rados -p mirektest stat rbd_directory mirektest/rbd_directory mtime 2020-10-07 11:55:16.000000, size 0 -> rados -p mirektest stat rbd_info mirektest/rbd_info mtime 2019-12-05 12:11:39.000000, size 19 -> rados -p mirektest get rbd_info - overwrite validated -> rados -p mirektest stat rbd_trash mirektest/rbd_trash mtime 2020-10-07 11:55:17.000000, size 0 Do any of you have some idea what's happening here? I am trying to find way how to cleanup pool without interrupting existing content in general. Also I have probably no idea how to replicate this and it's not the story of every pool in cluster. There is e.g. pool which was used for Kubernetes but it has no issues: -> rados df POOL_NAME        USED OBJECTS CLONES COPIES MISSING_ON_PRIMARY UNFOUND DEGRADED   RD_OPS   RD   WR_OPS   WR USED COMPR UNDER COMPR pool12       192 KiB    3   0    9         0    0    0  8710688 807 GiB  5490880 576 GiB    0 B     0 B I don't know how to check other pools, because there is data in them . How can I rely on quotas when in empty cluster there is already 13GiB data and 3.2k objects (which btw corresponds because 3200 * 4MiB = 12.8 GiB). Also is there any way how to calculate same number of objects/bytes Ceph is using to enforce quotas? Thanks for you time if you read it until the end, really appreciated that! :-) Any idea or hint is welcome. 

--  Miroslav Kalina 

Systems development specialist



