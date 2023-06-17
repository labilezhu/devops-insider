


## Mount namespaces and shared subtrees


### Shared subtrees



> https://lwn.net/Articles/689856/

Once the implementation of mount namespaces was completed, user-space programmers encountered a usability problem: mount namespaces provided *too much* isolation between namespaces. Suppose, for example, that a new disk is loaded into an optical disk drive. In the original implementation, the only way to make that disk visible in all mount namespaces was to mount the disk separately in each namespace. In many cases, it would instead be preferable to perform a single mount operation that makes the disk visible in all (or perhaps some subset) of the mount namespaces on the system.

Because of the problem just described, the [shared subtrees feature](https://lwn.net/Articles/159077/) was added in Linux 2.6.15 (in early 2006, around three years after the initial implementation of mount namespaces). **The key benefit of shared subtrees is to allow automatic, controlled propagation of mount and unmount events between namespaces.** This means, for example, that mounting an optical disk in one mount namespace can trigger a mount of that disk in all other namespaces.


Under the shared subtrees feature, each mount point is marked with a "propagation type", which determines whether mount points created and removed under this mount point are propagated to other mount points. There are four different propagation types:

-   `MS_SHARED`: This mount point shares mount and unmount events with other mount points that are members of its "peer group" (which is described in more detail below). When a mount point is added or removed under this mount point, this change will propagate to the peer group, so that the mount or unmount will also take place under each of the peer mount points. Propagation also occurs in the reverse direction, so that mount and unmount events on a peer mount will also propagate to this mount point.
-   `MS_PRIVATE`: This is the converse of a shared mount point. The mount point does not propagate events to any peers, and does not receive propagation events from any peers.
-   `MS_SLAVE`: This propagation type sits midway between shared and private. A slave mount has a master---a shared peer group whose members propagate mount and unmount events to the slave mount. However, the slave mount does not propagate events to the master peer group.
-   `MS_UNBINDABLE`: This mount point is unbindable. Like a private mount point, this mount point does not propagate events to or from peers. In addition, this mount point can't be the source for a bind mount operation.

需要注意：

- propagation type is a per-mount-point setting. Within a namespace, some mount points might be marked shared, while others are marked private (or slave or unbindable)
- propagation type determines the propagation of mount and unmount events **immediately under** the mount point

#### Peer groups
A peer group is a set of mount points that propagate mount and unmount events to one another. A peer group acquires new members when a mount point whose propagation type is shared is either replicated during the creation of a new namespace or is used as the source for a bind mount.

E.g

```
    sh1# mount --make-private /
    sh1# mount --make-shared /dev/sda3 /X
    sh1# mount --make-shared /dev/sda5 /Y
```

```
    sh2# unshare -m --propagation unchanged sh
```

```
    sh1# mkdir /Z
    sh1# mount --bind /X /Z
```

 Following these steps, we have the situation shown in the diagram below. 
![](ns-mount.assets/submount-eg.png)


In this scenario, there are two peer groups:

-   The first peer group contains the mount points X, X' (the duplicate of mount point X that was created when the second namespace was created), and Z (the bind mount created from the source mount point X in the initial namespace).
-   The second peer group contains the mount points Y and Y' (the duplicate of mount point Y that was created when the second namespace was created).

Note that the bind mount Z, which was created in the initial namespace *after* the second namespace was created, was not replicated in the second namespace because the parent mount (`/`) was marked private.

#### Examining propagation types and peer groups via /proc/PID/mountinfo

The `/proc/PID/mountinfo` file (documented in the `[proc(5)](http://man7.org/linux/man-pages/man5/proc.5.html)` manual page) displays a range of information about the mount points for the mount namespace in which the process `PID` resides. All processes that reside in the same mount namespace will see the same view in this file. This file was designed to provide more information about mount points than was possible with the older, non-extensible `/proc/PID/mounts` file. Included in each record in this file is a (possibly empty) set of so-called "optional fields", which display information about the propagation type and peer group (for shared mounts) of each mount.

For a shared mount, the optional fields in the corresponding record in `/proc/PID/mountinfo` will contain a tag of the form `shared:N`. Here, the `shared` tag indicates that the mount is sharing propagation events with a peer group. The peer group is identified by `N`, an integer value that uniquely identifies the peer group. **These IDs are numbered starting at 1, and may be recycled when a peer group ceases to exist because all of its members departed the group.** All mount points that are members of the same peer group will show a `shared:N` tag with the same `N` in the `/proc/PID/mountinfo` file.


Thus for example, if we list the contents of `/proc/self/mountinfo` in the first of the shells discussed in the example above, we see the following (with a little bit of `sed` filtering to trim some irrelevant information from the output):

```
    sh1# **cat /proc/self/mountinfo | sed 's/ - .*//'**
    61 0 8:2 / / rw,relatime
    81 61 8:3 / /X rw,relatime shared:1
    124 61 8:5 / /Y rw,relatime shared:2
    228 61 8:3 / /Z rw,relatime shared:1
```

From this output, we first see that the root mount point is private. This is indicated by the absence of any tags in the optional fields. We also see that the mount points `/X` and `/Z` are shared mount points in the same peer group (with ID 1), which means that mount and unmount events under either of these two mounts will propagate to the other. The mount `/Y` is a shared mount in a different peer group (ID 2), which, by definition, does not propagate events to or from the mounts in peer group 1.

The `/proc/PID/mountinfo` file also enables us to see the parental relationship between mount points. The first field in each record is a unique ID for each mount point. The second field is the ID for the parent mount. From the above output, we can see that the mount points `/X`, `/Y`, and `/Z` are all children of the root mount because their parent IDs are all 61.

Running the same command in the second shell (in the second namespace), we see:

```
    sh2# **cat /proc/self/mountinfo | sed 's/ - .*//'**
    147 146 8:2 / / rw,relatime
    221 147 8:3 / /X rw,relatime shared:1
    224 147 8:5 / /Y rw,relatime shared:2
```

Again, we see that the root mount point is private. Then we see that `/X` is a shared mount in peer group 1, the same peer group as the mounts `/X` and `/Z` in the initial mount namespace. Finally, we see that `/Y` is a shared mount in peer group 2, the same peer group as the mount `/Y` in the initial mount namespace. One final point to note is that the mount points that were replicated in the second namespace have their own unique IDs that differ from the IDs of the corresponding mounts in the initial namespace.

#### 默认的共享模式(Debating defaults)

Because the situation is a little complex, we have so far avoided discussing what the default propagation type is for a new mount point. From the kernel's perspective, the default when a new device mount is created is as follows:

-   If the mount point has a parent (i.e., it is a non-root mount point) and the propagation type of the parent is `MS_SHARED`, then the propagation type of the new mount is also `MS_SHARED`.
-   Otherwise, the propagation type of the new mount is `MS_PRIVATE`.

According to these rules, the root mount would be `MS_PRIVATE`, and all descendant mounts would by default also be `MS_PRIVATE`. However, `MS_SHARED` would arguably have been a better default, since it is the more commonly employed propagation type. For that reason, `systemd` sets the propagation type of all mount points to `MS_SHARED`. Thus, on most modern Linux distributions, the default propagation type is effectively `MS_SHARED`. This is not the final word on the subject, however, since the util-linux `unshare` utility also has something to say. When creating a new mount namespace, `unshare` assumes that the user wants a fully isolated namespace, and makes all mount points private by performing the equivalent of the following command (which recursively marks all mounts under the root directory as private):

```
    mount --make-rprivate /
```

To prevent this, we can use an additional option when creating the new namespace:

```
    unshare -m --propagation unchanged <cmd>
```

### Ref
> https://man7.org/linux/man-pages/man7/mount_namespaces.7.html

