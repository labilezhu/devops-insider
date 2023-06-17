---
tags:
- cloud
- kernel
- performance
- numa
---




### Monitor
https://www.kernel.org/doc/html/latest/admin-guide/numastat.html
/sys/devices/system/node/node*/numastat

In more detail:

| numa_hit       | A process wanted to allocate memory from this node, and succeeded.                              |
|----------------|-------------------------------------------------------------------------------------------------|
| numa_miss      | A process wanted to allocate memory from another node, but ended up with memory from this node. |
| numa_foreign   | A process wanted to allocate on this node, but ended up with memory from another node.          |
| local_node     | A process ran on this node’s CPU, and got memory from this node.                                |
| other_node     | A process ran on a different node’s CPU and got memory from this node.                          |
| interleave_hit | Interleaving wanted to allocate from this node and succeeded.                                   |


