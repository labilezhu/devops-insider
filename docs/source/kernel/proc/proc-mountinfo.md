


> https://man7.org/linux/man-pages/man5/proc.5.html

```
       */proc/[pid]/mountinfo* (since Linux 2.6.26)
              This file contains information about mounts in the
              process's mount namespace (see [mount_namespaces(7)](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html)).  It
              supplies various information (e.g., propagation state,
              root of mount for bind mounts, identifier for each mount
              and its parent) that is missing from the (older)
              */proc/[pid]/mounts* file, and fixes various other problems
              with that file (e.g., nonextensibility, failure to
              distinguish per-mount versus per-superblock options).

              The file contains lines of the form:

              36 35 98:0 /mnt1 /mnt2 rw,noatime master:1 - ext3 /dev/root rw,errors=continue
              (1)(2)(3)   (4)   (5)      (6)      (7)   (8) (9)   (10)         (11)

              The numbers in parentheses are labels for the descriptions
              below:

              (1)  mount ID: a unique ID for the mount (may be reused
                   after [umount(2)](https://man7.org/linux/man-pages/man2/umount.2.html)).

              (2)  parent ID: the ID of the parent mount (or of self for
                   the root of this mount namespace's mount tree).

                   If a new mount is stacked on top of a previous
                   existing mount (so that it hides the existing mount)
                   at pathname P, then the parent of the new mount is
                   the previous mount at that location.  Thus, when
                   looking at all the mounts stacked at a particular
                   location, the top-most mount is the one that is not
                   the parent of any other mount at the same location.
                   (Note, however, that this top-most mount will be
                   accessible only if the longest path subprefix of P
                   that is a mount point is not itself hidden by a
                   stacked mount.)

                   If the parent mount lies outside the process's root
                   directory (see [chroot(2)](https://man7.org/linux/man-pages/man2/chroot.2.html)), the ID shown here won't
                   have a corresponding record in *mountinfo* whose mount
                   ID (field 1) matches this parent mount ID (because
                   mounts that lie outside the process's root directory
                   are not shown in *mountinfo*).  As a special case of
                   this point, the process's root mount may have a
                   parent mount (for the initramfs filesystem) that lies
                   outside the process's root directory, and an entry
                   for that mount will not appear in *mountinfo*.

              (3)  major:minor: the value of *st_dev* for files on this
                   filesystem (see [stat(2)](https://man7.org/linux/man-pages/man2/stat.2.html)).

              (4)  root: the pathname of the directory in the filesystem
                   which forms the root of this mount.

              (5)  mount point: the pathname of the mount point relative
                   to the process's root directory.

              (6)  mount options: per-mount options (see [mount(2)](https://man7.org/linux/man-pages/man2/mount.2.html)).

              (7)  optional fields: zero or more fields of the form
                   "tag[:value]"; see below.

              (8)  separator: the end of the optional fields is marked
                   by a single hyphen.

              (9)  filesystem type: the filesystem type in the form
                   "type[.subtype]".

              (10) mount source: filesystem-specific information or
                   "none".

              (11) super options: per-superblock options (see [mount(2)](https://man7.org/linux/man-pages/man2/mount.2.html)).

              Currently, the possible optional fields are *shared*,
              *master*, *propagate_from*, and *unbindable*.  See
              [mount_namespaces(7)](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html) for a description of these fields.
              Parsers should ignore all unrecognized optional fields.

              For more information on mount propagation see:
              *Documentation/filesystems/sharedsubtree.txt* in the Linux
              kernel source tree.
```