---
tags:
- cloud
- kernel
- performance
---

#### Base cgroup files
> * tasks: list of tasks (by PID) attached to that cgroup. This list is not guaranteed to be sorted. Writing a thread ID into this file moves the thread into this cgroup.
>     
> * cgroup.procs: list of thread group IDs in the cgroup. This list is not guaranteed to be sorted or free of duplicate TGIDs, and userspace should sort/uniquify the list if this property is required. Writing a thread group ID into this file moves all threads in that group into this cgroup.
>     
> * notify\_on\_release flag: run the release agent on exit?
>     
> * release_agent: the path to use for release notifications (this file exists in the top cgroup only)




## Ref
https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/cgroups.html
