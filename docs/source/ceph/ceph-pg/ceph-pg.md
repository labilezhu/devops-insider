# ceph Placement Group (PG)

> https://docs.ceph.com/en/quincy/rados/operations/placement-groups/#placement-groups









## PEERING

> https://docs.ceph.com/en/quincy/dev/peering/

- *Peering*

the process of bringing all of the OSDs that store a Placement Group (PG) into agreement about the state of all of the objects (and their metadata) in that PG. Note that agreeing on the state does not mean that they all have the latest contents.

- *Acting set*

the ordered list of OSDs who are (or were as of some epoch) responsible for a particular PG.

- *Up set*

the ordered list of OSDs responsible for a particular PG for a particular epoch according to CRUSH. Normally this is the same as the *acting set*, except when the *acting set* has been explicitly overridden via *PG temp* in the OSDMap.