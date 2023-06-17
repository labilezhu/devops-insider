---
date: 2021-07-20T23:12:15+09:00
draft: true
---

## API

> https://man7.org/linux/man-pages/man7/inotify.7.html

*  [inotify_init(2)](https://man7.org/linux/man-pages/man2/inotify_init.2.html) creates an inotify instance and returns a file descriptor referring to the inotify instance. The more recent [inotify_init1(2)](https://man7.org/linux/man-pages/man2/inotify_init1.2.html) is like [inotify_init(2)](https://man7.org/linux/man-pages/man2/inotify_init.2.html), but has a _flags_ argument that provides access to some extra functionality.

*  [inotify_add_watch(2)](https://man7.org/linux/man-pages/man2/inotify_add_watch.2.html) manipulates the "watch list" associated with an inotify instance. Each item ("watch") in the watch list specifies the pathname of a file or directory, along with some set of events that the kernel should monitor for the file referred to by that pathname. [inotify_add_watch(2)](https://man7.org/linux/man-pages/man2/inotify_add_watch.2.html) either creates a new watch item, or modifies an existing watch. Each watch has a unique "watch descriptor", an integer returned by [inotify_add_watch(2)](https://man7.org/linux/man-pages/man2/inotify_add_watch.2.html) when the watch is created. 
  
* When events occur for monitored files and directories, those events are made available to the application as structured data that can be read from the inotify file descriptor using [read(2)](https://man7.org/linux/man-pages/man2/read.2.html) (see below). 
  
*  [inotify_rm_watch(2)](https://man7.org/linux/man-pages/man2/inotify_rm_watch.2.html) removes an item from an inotify watch list. 
  
* When all file descriptors referring to an inotify instance have been closed (using [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)), the underlying object and its resources are freed for reuse by the kernel; all associated watches are automatically freed.

### Reading events from an inotify file descriptor
To determine what events have occurred, an application [read(2)](https://man7.org/linux/man-pages/man2/read.2.html)s from the inotify file descriptor. If no events have so far occurred, then, assuming a blocking file descriptor, [read(2)](https://man7.org/linux/man-pages/man2/read.2.html) will block until at least one event occurs (unless interrupted by a signal, in which case the call fails with the error **EINTR**; see [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html)). Each successful [read(2)](https://man7.org/linux/man-pages/man2/read.2.html) returns a buffer containing one or more of the following structures: 

```c
           struct inotify_event {
               int      wd;       /* Watch descriptor */
               uint32_t mask;     /* Mask describing event */
               uint32_t cookie;   /* Unique cookie associating related
                                     events (for rename(2)) */
               uint32_t len;      /* Size of name field */
               char     name[];   /* Optional null-terminated name */
           };
```

### inotify events

```
           IN_ACCESS (+)
                  File was accessed (e.g., read(2), execve(2)).

           IN_ATTRIB (*)
                  Metadata changed—for example, permissions (e.g.,
                  chmod(2)), timestamps (e.g., utimensat(2)), extended
                  attributes (setxattr(2)), link count (since Linux
                  2.6.25; e.g., for the target of link(2) and for
                  unlink(2)), and user/group ID (e.g., chown(2)).

           IN_CLOSE_WRITE (+)
                  File opened for writing was closed.

           IN_CLOSE_NOWRITE (*)
                  File or directory not opened for writing was closed.

           IN_CREATE (+)
                  File/directory created in watched directory (e.g.,
                  open(2) O_CREAT, mkdir(2), link(2), symlink(2),
                  bind(2) on a UNIX domain socket).

           IN_DELETE (+)
                  File/directory deleted from watched directory.

           IN_DELETE_SELF
                  Watched file/directory was itself deleted.  (This
                  event also occurs if an object is moved to another
                  filesystem, since mv(1) in effect copies the file to
                  the other filesystem and then deletes it from the
                  original filesystem.)  In addition, an IN_IGNORED
                  event will subsequently be generated for the watch
                  descriptor.

           IN_MODIFY (+)
                  File was modified (e.g., write(2), truncate(2)).

           IN_MOVE_SELF
                  Watched file/directory was itself moved.

           IN_MOVED_FROM (+)
                  Generated for the directory containing the old
                  filename when a file is renamed.

           IN_MOVED_TO (+)
                  Generated for the directory containing the new
                  filename when a file is renamed.

           IN_OPEN (*)
                  File or directory was opened.
```

## mux

Inotify file descriptors can be monitored using select(2),
poll(2), and epoll(7).  When an event is available, the file
descriptor indicates as readable.



## Limit
### /proc interfaces

The following interfaces can be used to limit the amount of kernel memory consumed by inotify: 

- /proc/sys/fs/inotify/max_queued_events
  - The value in this file is used when an application calls [inotify_init(2)](https://man7.org/linux/man-pages/man2/inotifyinit.2.html) to set an upper limit on the number of events that can be queued to the corresponding inotify instance. Events in excess of this limit are dropped, but an **IN_Q_OVERFLOW** event is always generated. 
- /proc/sys/fs/inotify/max_user_instances 
  - This specifies an upper limit on the number of inotify instances that can be created per real user ID. 
- /proc/sys/fs/inotify/max_user_watches 
  - This specifies an upper limit on the number of watches that can be created per real user ID.

### event queue can overflow
       Note that the event queue can overflow.  In this case, events are
       lost.  Robust applications should handle the possibility of lost
       events gracefully.  For example, it may be necessary to rebuild
       part or all of the application cache.  (One simple, but possibly
       expensive, approach is to close the inotify file descriptor,
       empty the cache, create a new inotify file descriptor, and then
       re-create watches and cache entries for the objects to be
       monitored.)


## Inspect

### Checking inotify Users
> https://www.baeldung.com/linux/inotify-upper-limit-reached

```bash
$ inotifywatch / &
Establishing watches...
Finished establishing watches, now collecting statistics.
[1] 666
$ find /proc/*/fd -lname anon_inode:inotify |
  cut -d/ -f3 |
  xargs -I '{}' -- ps --no-headers -o '%p %U %c' -p '{}' |
  uniq -c |
  sort -nr
      1    6660 root     inotifywatch
```

```bash
cd /proc/$?/fd
ls -l
lr-x------ 1 labile labile 64 5月  15 07:18 22 -> anon_inode:inotify


cd /proc/$?/fdinfo

fdinfo find . -type f 2>/dev/null | xargs cat

inotify wd:3 ino:3ad sdev:19 mask:3cc ignored_mask:0 fhandle-bytes:8 fhandle-type:1 f_handle:80fbf87fad030000
inotify wd:2 ino:280001 sdev:800012 mask:3cc ignored_mask:0 fhandle-bytes:8 fhandle-type:1 f_handle:01002800959902e7
inotify wd:1 ino:2 sdev:800012 mask:3cc ignored_mask:0 fhandle-bytes:8 fhandle-type:1 f_handle:0200000000000000
```


### inotify-consumers
> https://github.com/fatso83/dotfiles/blob/master/utils/scripts/inotify-consumers
```bash
#!/bin/bash

# Get the procs sorted by the number of inotify watches
# @author Carl-Erik Kopseng
# @latest https://github.com/fatso83/dotfiles/blob/master/utils/scripts/inotify-consumers
# Discussion leading up to answer: https://unix.stackexchange.com/questions/15509/whos-consuming-my-inotify-resources
# Speed enhancements by Simon Matter <simon.matter@invoca.ch>

main(){
    # get terminal width
    declare -i COLS=$(tput cols 2>/dev/null || echo 80)
    declare -i WLEN=10

    printf "\n%${WLEN}s  %${WLEN}s\n" "INOTIFY" "INSTANCES"
    printf "%${WLEN}s  %${WLEN}s\n" "WATCHES" "PER   "
    printf "%${WLEN}s  %${WLEN}s  %s\n" " COUNT " "PROCESS "    "PID USER         COMMAND"
    printf -- "------------------------------------------------------------\n"
    generateData
}

usage(){
    cat << EOF
Usage: $0 [--help|--limits]

    -l, --limits    Will print the current related limits and how to change them
    -h, --help      Show this help
EOF
}

limits(){
    printf "\nCurrent limits\n-------------\n"
    sysctl fs.inotify.max_user_instances fs.inotify.max_user_watches

    cat <<- EOF


Changing settings permanently
-----------------------------
echo fs.inotify.max_user_watches=524288 | sudo tee -a /etc/sysctl.conf
sudo sysctl -p # re-read config

EOF
}

if [ "$1" = "--limits" -o "$1" = "-l" ]; then
    limits
    exit 0
fi

if [ "$1" = "--help" -o "$1" = "-h" ]; then
    usage 
    exit 0
fi

if [ -n "$1" ]; then
    printf "\nUnknown parameter '$1'\n" >&2
    usage
    exit 1
fi

generateData(){
    local -i PROC
    local -i PID
    local -i CNT
    local -i INSTANCES
    local -i TOT
    local -i TOTINSTANCES
    # read process list into cache
    local PSLIST="$(ps ax -o pid,user=WIDE-COLUMN,command --columns $(( COLS - WLEN )))"
    local INOTIFY="$(find /proc/[0-9]*/fdinfo -type f 2>/dev/null | xargs grep ^inotify 2>/dev/null)"
    local INOTIFYCNT="$(echo "$INOTIFY" | cut -d "/" -s --output-delimiter=" "  -f 3 |uniq -c | sed -e 's/:.*//')"
    # unique instances per process is denoted by number of inotify FDs
    local INOTIFYINSTANCES="$(echo "$INOTIFY" | cut -d "/" -s --output-delimiter=" "   -f 3,5 | sed -e 's/:.*//'| uniq |awk '{print $1}' |uniq -c)"
    local INOTIFYUSERINSTANCES="$(echo "$INOTIFY" | cut -d "/" -s --output-delimiter=" "   -f 3,5 | sed -e 's/:.*//' | uniq |
    	     while read PID FD; do echo $PID $FD $(grep -e "^\ *${PID}\ " <<< "$PSLIST"|awk '{print $2}'); done | cut -d" "  -f 3 | sort | uniq -c |sort -nr)"
    set -e

    cat <<< "$INOTIFYCNT" |
        {
            while read -rs CNT PROC; do   # count watches of processes found
                echo "${PROC},${CNT},$(echo "$INOTIFYINSTANCES" | grep " ${PROC}$" |awk '{print $1}')"
            done
        } |
        grep -v ",0," |                  # remove entires without watches
        sort -n -t "," -k 2,3 -r |         # sort to begin with highest numbers
        {                                # group commands so that $TOT is visible in the printf
	    IFS=","
            while read -rs PID CNT INSTANCES; do   # show watches and corresponding process info
                printf "%$(( WLEN - 2 ))d  %$(( WLEN - 2 ))d     %s\n" "$CNT" "$INSTANCES" "$(grep -e "^\ *${PID}\ " <<< "$PSLIST")"
                TOT=$(( TOT + CNT ))
		TOTINSTANCES=$(( TOTINSTANCES + INSTANCES))
            done
	    # These stats should be per-user as well, since inotify limits are per-user..
            printf "\n%$(( WLEN - 2 ))d  %s\n" "$TOT" "WATCHES TOTAL COUNT"
# the total across different users is somewhat meaningless, not printing for now.
#            printf "\n%$(( WLEN - 2 ))d  %s\n" "$TOTINSTANCES" "TOTAL INSTANCES COUNT"
        }
    echo ""
    echo "INotify instances per user (e.g. limits specified by fs.inotify.max_user_instances): "
    echo ""
    (
      echo "INSTANCES    USER"
      echo "-----------  ------------------"
      echo "$INOTIFYUSERINSTANCES"
    ) | column -t
    echo ""

}

main
```

output e.g:
```log
   INOTIFY
   WATCHER
    COUNT     PID     CMD
----------------------------------------
    6688    27262  /home/dvlpr/apps/WebStorm-2018.3.4/WebStorm-183.5429.34/bin/fsnotifier64
     411    27581  node /home/dvlpr/dev/kiwi-frontend/node_modules/.bin/webpack --config config/webpack.dev.js
      79     1541  /usr/lib/gnome-settings-daemon/gsd-xsettings
      30     1664  /usr/lib/gvfs/gvfsd-trash --spawner :1.22 /org/gtk/gvfs/exec_spaw/0
      14     1630  /usr/bin/gnome-software --gapplication-service
    ....

    7489  watches TOTAL COUNT
```

> describe:
> https://unix.stackexchange.com/questions/15509/whos-consuming-my-inotify-resources

#### other inspect tool
> https://github.com/mikesart/inotify-info

## inode to path

> https://stackoverflow.com/questions/43165762/how-to-get-file-contents-by-inode-in-bash
> https://www.linux.org/docs/man8/debugfs.html

```bash
inode=11845768
device=/dev/sdb2
sudo debugfs -R "ncheck $inode" $device

debugfs 1.45.5 (07-Jan-2020)
Inode   Pathname
11845768        /nfs/shareset/home/blog
```

## Ref
[inotify 资源耗尽](https://imroc.cc/kubernetes/troubleshooting/node/runnig-out-of-inotify-watches.html)

###### 查看进程的 inotify watch 情况
```bash
#!/usr/bin/env bash
#
# Copyright 2019 (c) roc
#
# This script shows processes holding the inotify fd, alone with HOW MANY directories each inotify fd watches(0 will be ignored).
total=0
result="EXE PID FD-INFO INOTIFY-WATCHES\n"
while read pid fd; do \
  exe="$(readlink -f /proc/$pid/exe || echo n/a)"; \
  fdinfo="/proc/$pid/fdinfo/$fd" ; \
  count="$(grep -c inotify "$fdinfo" || true)"; \
  if [ $((count)) != 0 ]; then
    total=$((total+count)); \
    result+="$exe $pid $fdinfo $count\n"; \
  fi
done <<< "$(lsof +c 0 -n -P -u root|awk '/inotify$/ { gsub(/[urw]$/,"",$4); print $2" "$4 }')" && echo "total $total inotify watches" && result="$(echo -e $result|column -t)\n" && echo -e "$result" | head -1 && echo -e "$result" | sed "1d" | sort -k 4rn;
```
example output:

```log
total 1288 inotify watches
EXE                                                                                                        PID   FD-INFO                INOTIFY-WATCHES
/var/lib/rancher/k3s/data/21cd0fe0793c37721f0754fa9848fd7389b6a2ee136f48a17c4a8c8d30755c7c/bin/k3s         2246  /proc/2246/fdinfo/289  1138
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/6       56
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/54      6
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/55      6
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/51      5
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/52      5
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/56      5
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/18      4
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/48      4
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/53      4
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/57      4
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/59      4
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/61      4
/usr/bin/udevadm                                                                                           457   /proc/457/fdinfo/8     3
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/58      3
/usr/lib/systemd/systemd                                                                                   1     /proc/1/fdinfo/60      3
/usr/sbin/cron                                                                                             1982  /proc/1982/fdinfo/5    3

```

## My Rlated Blog 
[content/zh/notes/cloud/envoy/sds/envoy-sds.md](/content/zh/notes/cloud/envoy/sds/envoy-sds.md)
[content/zh/notes/cloud/k8s/secret/secret.md](/content/zh/notes/cloud/k8s/secret/secret.md)