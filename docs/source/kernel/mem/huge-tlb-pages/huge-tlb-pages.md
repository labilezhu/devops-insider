
### Stat
The `/proc/meminfo` file provides information about the total number of persistent hugetlb pages in the kernel’s huge page pool. It also displays default huge page size and information about the number of free, reserved and surplus huge pages in the pool of huge pages of default size. The huge page size is needed for generating the proper alignment and size of the arguments to system calls that map huge page regions.

The output of `cat /proc/meminfo` will include lines like:

```
HugePages_Total: uuu
HugePages_Free:  vvv
HugePages_Rsvd:  www
HugePages_Surp:  xxx
Hugepagesize:    yyy kB
Hugetlb:         zzz kB
```

where:

**HugePages_Total**

is the size of the pool of huge pages.

**HugePages_Free**

is the number of huge pages in the pool that are not yet allocated.

**HugePages_Rsvd**

is short for “reserved,” and is the number of huge pages for which a commitment to allocate from the pool has been made, but no allocation has yet been made. Reserved huge pages guarantee that an application will be able to allocate a huge page from the pool of huge pages at fault time.

**HugePages_Surp(过剩的 HugePage)**

is short for “surplus,” and is the number of huge pages in the pool above the value in `/proc/sys/vm/nr_hugepages`. The maximum number of surplus huge pages is controlled by `/proc/sys/vm/nr_overcommit_hugepages`. Note: When the feature of freeing unused vmemmap pages associated with each hugetlb page is enabled, the number of surplus huge pages may be temporarily larger than the maximum number of surplus huge pages when the system is under memory pressure.

**Hugepagesize**

is the default hugepage size (in Kb).

**Hugetlb**

is the total amount of memory (in kB), consumed by huge pages of all sizes. If huge pages of different sizes are in use, this number will exceed HugePages_Total * Hugepagesize. To get more detailed information, please, refer to `/sys/kernel/mm/hugepages` (described below).


`/proc/filesystems` should also show a filesystem of type “hugetlbfs” configured in the kernel.

#### /sys/kernel/mm/hugepages
For each huge page size supported by the running kernel, a subdirectory will exist, of the form:
```
 hugepages-${size}kB
```
Inside each of these directories, the same set of files will exist:
```
nr_hugepages
nr\_hugepages\_mempolicy
nr\_overcommit\_hugepages
free_hugepages
resv_hugepages
surplus_hugepages
```



##### NUMA hugepages stat
Administrators can verify the number of huge pages actually allocated by checking the sysctl or meminfo. **To check the per node distribution of huge pages in a NUMA system**, use:
```bash
cat /sys/devices/system/node/node*/meminfo | fgrep Huge
```


```bash
ls /sys/devices/system/node/node\[0-9\]*/hugepages/
nr_hugepages
free_hugepages
surplus_hugepages
```

The free_’ and surplus_’ attribute files are read-only. They return the number of free and surplus \[overcommitted\] huge pages, respectively, on the parent node.

The `nr_hugepages` attribute returns the total number of huge pages on the specified node. When this attribute is written, the number of persistent huge pages on the parent node will be adjusted to the specified value, if sufficient resources exist, regardless of the task’s mempolicy or cpuset constraints.




### Config

##### 内核启动时指定配置
The administrator can allocate persistent huge pages on the kernel boot command line by specifying the “hugepages=N” parameter, where ‘N’ = the number of huge pages requested. This is the most reliable method of allocating huge pages as memory has not yet become fragmented.

Some platforms support multiple huge page sizes. To allocate huge pages of a specific size, one must precede the huge pages boot command parameters with a huge page size selection parameter “hugepagesz=&lt;size&gt;”. &lt;size&gt; must be specified in bytes with optional scale suffix \[kKmMgG\]. The default huge page size may be selected with the “default_hugepagesz=&lt;size&gt;” boot parameter.

Hugetlb boot command line parameter semantics

##### 内核启动后修改配置

`/proc/sys/vm/nr_hugepages` indicates the current number of “persistent” huge pages in the kernel’s huge page pool. “Persistent” huge pages will be returned to the huge page pool when freed by a task. A user with root privileges can dynamically allocate more or free some persistent huge pages by increasing or decreasing the value of `nr_hugepages`.

`/proc/sys/vm/nr_hugepages` indicates the current number of pre-allocated huge pages of the default size. Thus, one can use the following command to dynamically allocate/deallocate default sized persistent huge pages:

```
 echo 20 > /proc/sys/vm/nr_hugepages
```

This command will try to adjust the number of default sized huge pages in the huge page pool to 20, allocating or freeing huge pages, as required.

On a NUMA platform, the kernel will attempt to distribute the huge page pool over all the set of allowed nodes specified by the NUMA memory policy of the task that modifies `nr_hugepages`.(当进程写 `nr_hugepages` 文件时，新分配的 huge pages 只分配到写进程所允许使用的 node。所以，一般在 local rc init files 中，系统初始化时分配 huge pages) The default for the allowed nodes–when the task has default memory policy–is all on-line nodes with memory. Allowed nodes with insufficient available, contiguous memory for a huge page will be silently skipped when allocating persistent huge pages. 

##### overcommit
`/proc/sys/vm/nr_overcommit_hugepages` specifies how large the pool of huge pages can grow, if more huge pages than `/proc/sys/vm/nr_hugepages` are requested by applications. Writing any non-zero value into this file indicates that the hugetlb subsystem is allowed to try to obtain that number of “surplus” huge pages from the kernel’s normal page pool, when the persistent huge page pool is exhausted. As these surplus huge pages become unused, they are freed back to the kernel’s normal page pool.

##### shrink
The administrator may shrink the pool of persistent huge pages for the default huge page size by setting the `nr_hugepages` sysctl to a smaller value. The kernel will attempt to balance the freeing of huge pages across all nodes in the memory policy of the task modifying `nr_hugepages`(影响的 node 和写配置文件的进程有关). Any free huge pages on the selected nodes will be freed back to the kernel’s normal page pool.


### Limit
**huge pages 内存不能被非 huge page 使用**
Pages that are used as huge pages are reserved inside the kernel and cannot be used for other purposes. Huge pages cannot be swapped out under memory pressure.




## NUMA and huge-pages
On a NUMA platform, the kernel will attempt to distribute the huge page pool over all the set of allowed nodes specified by the NUMA memory policy of the task that modifies `nr_hugepages`.(当进程写 `nr_hugepages` 文件时，新分配的 huge pages 只分配到写进程所允许使用的 node) The default for the allowed nodes–when the task has default memory policy–is all on-line nodes with memory. Allowed nodes with insufficient available, contiguous memory for a huge page will be silently skipped when allocating persistent huge pages. See the [discussion below](https://www.kernel.org/doc/html/latest/admin-guide/mm/hugetlbpage.html#mem-policy-and-hp-alloc) of the interaction of task memory policy, cpusets and per node attributes with the allocation and freeing of persistent huge pages.

The success or failure of huge page allocation depends on the amount of physically contiguous memory that is present in system at the time of the allocation attempt. If the kernel is unable to allocate huge pages from some nodes in a NUMA system, it will attempt to make up the difference by allocating extra pages on other nodes with sufficient available contiguous memory, if any.

System administrators may want to put this command in one of the local rc init files. This will enable the kernel to allocate huge pages early in the boot process when the possibility of getting physical contiguous pages is still very high. Administrators can verify the number of huge pages actually allocated by checking the sysctl or meminfo. To check the per node distribution of huge pages in a NUMA system, use:

```bash
cat /sys/devices/system/node/node*/meminfo | fgrep Huge
```


### Using Huge Pages

If the user applications are going to request huge pages using `mmap` system call, then it is required that system administrator mount a file system of type hugetlbfs:

```bash
 mount -t hugetlbfs \
      -o uid=&lt;value&gt;,gid=&lt;value&gt;,mode=&lt;value&gt;,pagesize=&lt;value&gt;,size=&lt;value&gt;,\
      min\_size=&lt;value&gt;,nr\_inodes=&lt;value&gt; none /mnt/huge
```


### Ref
https://www.kernel.org/doc/html/latest/admin-guide/mm/hugetlbpage.html