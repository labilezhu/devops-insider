---
tags:
- cloud
- kernel
- performance
---

## What are cpusets ?
Cpusets provide a mechanism for assigning a set of CPUs and Memory Nodes to a set of tasks. In this document “Memory Node” refers to an on-line node that contains memory.

Cpusets constrain the CPU and Memory placement of tasks to only the resources within a task’s current cpuset. They form a nested hierarchy visible in a virtual file system. These are the essential **hooks**, beyond what is already present, required to manage dynamic job placement on large systems.

Cpusets usthe generic cgroup subsystem described in [Control Groups](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/cgroups.html).

Requests by a task, using the sched\_setaffinity(2) system call to include CPUs in its CPU affinity mask, and using the mbind(2) and set\_mempolicy(2) system calls to include Memory Nodes in its memory policy, are both filtered through that task’s cpuset, filtering out any CPUs or Memory Nodes not in that cpuset.The scheduler will not schedule a task on a CPU that is not allowed in its `cpus_allowed` vector, and the kernel page allocator will not allocate a page on a node that is not allowed in the requesting task’s mems\_allowed vector.

User level code may create and destroy cpusets by name in the cgroup virtual file system, manage the attributes and permissions of these cpusets and which CPUs and Memory Nodes are assigned to each cpuset, specify and query to which cpuset a task is assigned, and list the task pids assigned to a cpuset.


### How are cpusets implemented ?
Cpusets provide a Linux kernel mechanism to constrain which CPUs and Memory Nodes are used by a process or set of processes.

The Linux kernel already has a pair of mechanisms to specify on which CPUs a task may be scheduled (`sched_setaffinity`) and on which Memory Nodes it may obtain memory (`mbind`, `set_mempolicy`).

Cpusets extends these two mechanisms as follows:

> * Cpusets are sets of allowed CPUs and Memory Nodes, known to the kernel.
> * Each task in the system is attached to a cpuset, via a pointer in the task structure to a reference counted cgroup structure.
> * Calls to sched_setaffinity are filtered to just those CPUs allowed in that task’s cpuset.
> * Calls to mbind and set_mempolicy are filtered to just those Memory Nodes allowed in that task’s cpuset.
> * The root cpuset contains all the systems CPUs and Memory Nodes.
> * For any cpuset, one can define child cpusets containing a subset of the parents CPU and Memory Node resources.
> * The hierarchy of cpusets can be mounted at /dev/cpuset, for browsing and manipulation from user space.
> * **A cpuset may be marked exclusive, which ensures that no other cpuset (except direct ancestors and descendants) may contain any overlapping CPUs or Memory Nodes.**
> * You can list all the tasks (by pid) attached to any cpuset.

### task stat
```bash
cat /proc/<pid>/status

Cpus_allowed:   ffffffff,ffffffff,ffffffff,ffffffff
Cpus\_allowed\_list:      0-127
Mems_allowed:   ffffffff,ffffffff
Mems\_allowed\_list:      0-63
```

### cgroup config
```
        cpuset.cpus: list of CPUs in that cpuset

        cpuset.mems: list of Memory Nodes in that cpuset

        cpuset.memory_migrate flag: if set, move pages to cpusets nodes

        cpuset.cpu_exclusive flag: is cpu placement exclusive?

        cpuset.mem_exclusive flag: is memory placement exclusive?

        cpuset.mem_hardwall flag: is memory allocation hardwalled

        cpuset.memory_pressure: measure of how much paging pressure in cpuset

        cpuset.memory_spread_page flag: if set, spread page cache evenly on allowed nodes

        cpuset.memory_spread_slab flag: if set, spread slab cache evenly on allowed nodes

        cpuset.sched_load_balance flag: if set, load balance within CPUs on that cpuset

        cpuset.sched_relax_domain_level: the searching range when migrating tasks
```

#### Memory allocation & Kernel memory
A cpuset that is cpuset.mem_exclusive _or_ cpuset.mem\_hardwall is “hardwalled”, i.e. it restricts kernel allocations for page, buffer and other data commonly shared by the kernel across multiple users. All cpusets, whether hardwalled or not, restrict allocations of memory for user space. This enables configuring a system so that several independent jobs can share common kernel data, such as file system pages, while isolating each job’s user allocation in its own cpuset. To do this, construct a large mem\_exclusive cpuset to hold all the jobs, and construct child, non-mem\_exclusive cpusets for each individual job. Only a small amount of typical kernel memory, such as requests from interrupt handlers, is allowed to be taken outside even a mem\_exclusive cpuset.


### Task sched load balance
The kernel scheduler (kernel/sched/core.c) automatically load balances tasks. If one CPU is underutilized, kernel code running on that CPU will look for tasks on other more overloaded CPUs and move those tasks to itself, within the constraints of such placement mechanisms as cpusets and sched_setaffinity.


#### sched domain

##### why sched domain
The algorithmic cost of load balancing and its impact on key shared kernel data structures such as the task list increases more than linearly with the number of CPUs being balanced. So the scheduler has support to partition the systems CPUs into a number of sched domains such that it only load balances within each sched domain. Each `sched domain` covers some subset of the CPUs in the system; no two sched domains overlap; some CPUs might not be in any `sched domain` and hence won’t be load balanced.

Put simply, it costs less to balance between two smaller sched domains than one big one, but doing so means that overloads in one of the two domains won’t be load balanced to the other one.

By default, there is one `sched domain` covering all CPUs, including those marked isolated using the kernel boot time “isolcpus=” argument. However, the isolated CPUs will not participate in load balancing, and will not have tasks running on them unless explicitly assigned.

##### sched domain and cpuset

When the per-cpuset flag “cpuset.sched\_load\_balance” is enabled (the default setting), it requests that all the CPUs in that cpusets allowed ‘cpuset.cpus’ be contained in a single `sched domain`, ensuring that load balancing can move a task (not otherwised pinned, as by sched_setaffinity) from any CPU in that cpuset to any other.

When the per-cpuset flag “cpuset.sched\_load\_balance” is disabled, then the scheduler will avoid load balancing across the CPUs in that cpuset, –except– in so far as is necessary because some overlapping cpuset has “sched\_load\_balance” enabled.

So, for example, if the top cpuset has the flag “cpuset.sched\_load\_balance” enabled, then the scheduler will have one `sched domain` covering all CPUs, and the setting of the “cpuset.sched\_load\_balance” flag in any other cpusets won’t matter, as we’re already fully load balancing.

**Therefore in the above two situations, the top cpuset flag “cpuset.sched\_load\_balance” should be disabled, and only some of the smaller, child cpusets have this flag enabled.**

There is an impedance mismatch here(这里有一个悖论), between `cpusets` and `sched domains`. Cpusets are hierarchical and nest. Sched domains are flat; they don’t overlap and each CPU is in at most one sched domain.

### How suched and balance
In sched domain, the scheduler migrates tasks in 2 ways; periodic load balance on tick, and at time of some schedule events.

**硬件亲和balance：**
When a task is woken up, scheduler try to move the task on idle CPU. For example, if a task A running on CPU X activates another task B on the same CPU X, and if CPU Y is X’s sibling and performing idle, then scheduler migrate task B to CPU Y so that task B can start on CPU Y without waiting task A on CPU X.

**空闲的CPU主动帮忙:**
And if a CPU run out of tasks in its runqueue, the CPU try to pull extra tasks from other busy CPUs to help them before it is going to be idle.




## Ref.

https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/cpusets.html
