# PID Namespace 的 Main process 结束 与 zombie child process

## 问题
挂载 PV 的 Pod 长期停留在 Terminating 状态。Linux top 命令显示高于 20% 的 iowait。

## 情况调查


### PID Namespace pid tree

```
root     15918     1  0 11月03 ?      00:15:32 /usr/local/bin/containerd-shim-runc-v2 -namespace k8s.io -id 7a6d383e2794ab23f2a070 -address /run/containerd/containerd.sock
65535    15939 15918  0 11月03 ?      00:00:00  \_ /pause
47040    22026 15918  0 11月03 ?      00:00:00  \_ [catatonit]
47040    22103 22026  9 11月03 ?      08:59:39  |   \_ [prometheus] <defunct>

```


```
root     15918     1  0 Nov03 ?        00:15:35 /usr/local/bin/containerd-shim-runc-v2 -namespace k8s.io -id 7a6d383e2794ab23f2a070 -address /run/containerd/containerd.sock
65535    15939 15918  0 Nov03 ?        00:00:00  \_ /pause
47040    22026 15918  0 Nov03 ?        00:00:00  \_ [catatonit] 
47040    22103 22026  9 Nov03 ?        08:59:39  |   \_ [prometheus] <defunct>
47040    22267 15918  0 Nov03 ?        00:00:00  \_ /usr/bin/catatonit -- /bin/bash -c /initenv
47040    22314 22267  0 Nov03 ?        00:16:42  |   \_ /bin/reverseproxy
47040    22531 15918  0 Nov03 ?        00:00:00  \_ /usr/bin/catatonit -- /bin/bash -c /initenv --web.listen-address=0.0.0.0:9085 --volume-dir=/etc/config --webhook-url=http://127.0.0.1:9090/-/reload
47040    22578 22531  0 Nov03 ?        00:01:00  |   \_ /bin/configmap-reload --web.listen-address=0.0.0.0:9085 --volume-dir=/etc/config --webhook-url=http://127.0.0.1:9090/-/reload
47040    22787 15918  0 Nov03 ?        00:00:00  \_ /usr/bin/catatonit -- /bin/bash -c /initenv --service-id=com-pm-server --container-name=com-pm-exporter
47040    22859 22787  0 Nov03 ?        00:01:11      \_ /bin/metrics-exporter --service-id=com-pm-server --container-name=com-pm-exporter

## 上面进程 command 带中括号原因：Sometimes the process args will be unavailable; when this happens, ps will instead print the executable name in brackets.

host22055:/proc/22103 # cat stat
22103 (prometheus) Z 22026 22103 22026 0 -1 4228356 20114323 0 227 0 3011373 226613 0 0 20 0 6 0 17708534 0 0 18446744073709551615 0 0 0 0 0 0 1002060288 0 2143420159 0 0 0 17 7 0 0 148 0 0 0 0 0 0 0 0 0 9
host22055:/proc/22103 # cat ../22026/stat
22026 (catatonit) S 15918 22026 22026 0 -1 4195588 481 0 2 0 0 0 0 0 20 0 1 0 17708523 0 0 18446744073709551615 0 0 0 0 0 0 1073478151 65536 0 1 0 0 17 3 0 0 1 0 0 0 0 0 0 0 0 0 9
host22055:/proc/22103 # cat /proc/15939/stat
15939 (pause) S 15918 15939 15939 0 -1 4194560 484 0 0 0 15 1 0 0 20 0 1 0 17700303 1019904 1 18446744073709551615 4194304 4722625 140732205646672 0 0 0 0 0 81922 1 0 0 17 1 0 0 0 0 0 4885568 4907536 8134656 140732205653921 140732205653928 140732205653928 140732205654001 0

```

```
user@host22055:~> sudo cat /proc/22026/stack 
[<0>] do_wait+0x1bf/0x230
[<0>] kernel_wait4+0x8d/0x140
[<0>] zap_pid_ns_processes+0x103/0x1b0
[<0>] do_exit+0xa44/0xb80
[<0>] do_group_exit+0x3a/0xa0
[<0>] get_signal+0x14a/0x850
[<0>] do_signal+0x36/0x6d0
[<0>] exit_to_usermode_loop+0x8b/0x120
[<0>] do_syscall_64+0x1b9/0x1e0
[<0>] entry_SYSCALL_64_after_hwframe+0x44/0xa9

```

[catatonit 源码](https://github.com/openSUSE/catatonit/blob/main/catatonit.c#L538)


#### zap_pid_ns_processes 卡的问题调查




- [pid: Improve the comment about waiting in zap_pid_ns_processes](http://kernsec.org/pipermail/linux-security-module-archive/2020-February/018848.html)

- [PID1 in containers - zap_pid_ns_processes()](https://www.marcusfolkesson.se/blog/pid1-in-containers/)

内核函数：
 - [do_group_exit](https://elixir.bootlin.com/linux/v4.4/source/kernel/exit.c#L855)
 - [do_exit](https://elixir.bootlin.com/linux/v4.4/source/kernel/exit.c#L474)
 - [exit_notify](https://elixir.bootlin.com/linux/v4.4/source/kernel/exit.c#L596)
 - [forget_original_parent](https://elixir.bootlin.com/linux/v4.4/source/kernel/exit.c#L560)
 - [find_child_reaper](https://elixir.bootlin.com/linux/v4.4/source/kernel/exit.c#L474)
 - [zap_pid_ns_processes](https://elixir.bootlin.com/linux/v4.4/source/kernel/pid_namespace.c#L184)
 - [zap_pid_ns_processes on v5.4](https://elixir.bootlin.com/linux/v5.4/source/kernel/pid_namespace.c#L236)


### IO 相关线程 Uninterruptible Sleep (D) 状态

#### 直接原因 Ceph full

```
$ dmesg -T
[Mon Nov  7 06:45:09 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:09 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:09 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:15 2022] __submit_request: 19 callbacks suppressed
[Mon Nov  7 06:45:15 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:15 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:15 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:15 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:15 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:27 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:32 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:39 2022] libceph: FULL or reached pool quota
[Mon Nov  7 06:45:46 2022] libceph: FULL or reached pool quota
```

#### top 状态

```
top -p 8912 -H


  PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND      
14096 root      20   0 7400724 1.793g  16108 R 3.667 5.719   0:20.65 vmstorage    
 8912 root      20   0 7400724 1.793g  16108 D 0.000 5.719  67:48.24 vmstorage    
 8944 root      20   0 7400724 1.793g  16108 S 0.000 5.719  27:34.23 vmstorage    
 8945 root      20   0 7400724 1.793g  16108 D 0.000 5.719 130:01.68 vmstorage    
 8946 root      20   0 7400724 1.793g  16108 S 0.000 5.719   0:00.01 vmstorage    
 8947 root      20   0 7400724 1.793g  16108 D 0.000 5.719 131:55.40 vmstorage    
 8948 root      20   0 7400724 1.793g  16108 S 0.000 5.719   0:00.00 vmstorage    
 8953 root      20   0 7400724 1.793g  16108 S 0.000 5.719   0:00.00 vmstorage    
12197 root      20   0 7400724 1.793g  16108 D 0.000 5.719 142:21.44 vmstorage    
12459 root      20   0 7400724 1.793g  16108 D 0.000 5.719 139:42.84 vmstorage    
12768 root      20   0 7400724 1.793g  16108 D 0.000 5.719 147:45.47 vmstorage    
19520 root      20   0 7400724 1.793g  16108 D 0.000 5.719 109:27.62 vmstorage    
27824 root      20   0 7400724 1.793g  16108 D 0.000 5.719  55:02.46 vmstorage    
14097 root      20   0 7400724 1.793g  16108 D 0.000 5.719   0:00.04 vmstorage    
14115 root      20   0 7400724 1.793g  16108 D 0.000 5.719   0:01.82 vmstorage    
14161 root      20   0 7400724 1.793g  16108 S 0.000 5.719   0:01.88 vmstorage    


```

#### 列出所有 D 状态的线程

> [Processes in an Uninterruptible Sleep (D) State](https://www.suse.com/support/kb/doc/?id=000016919)

```
top - 07:45:10 up 5 days, 20:33,  1 user,  load average: 26.22, 26.56, 25.38
Tasks: 479 total,   1 running, 474 sleeping,   0 stopped,   4 zombie
%Cpu(s):  8.5 us,  3.9 sy,  0.0 ni, 32.1 id, 55.1 wa,  0.0 hi,  0.4 si,  0.0 st
MiB Mem : 32106.83+total,  870.090 free, 17359.10+used, 14639.44+buff/cache
MiB Swap:    0.000 total,    0.000 free,    0.000 used. 14747.73+avail Mem 




host22055:/proc/22103 # ps -eo ppid,pid,user,stat,pcpu,comm,wchan:32 | grep " D"
    2    67 root     DN    0.0 khugepaged      khugepaged
    2  4858 root     D     0.0 xfsaild/rbd1    -
    2  7620 root     D     0.0 xfsaild/rbd3    -
 8541  8912 root     Dsl  11.2 vmstorage       lookup_slow
    2 12572 root     D     0.0 kworker/u16:1+f -
    2 15842 root     D     0.0 xfsaild/rbd0    -
    2 26317 root     D     0.0 kworker/u16:0+f -

```


#### 列表所有 D 状态的内核 statck

> [Processes in an Uninterruptible Sleep (D) State](https://www.suse.com/support/kb/doc/?id=000016919)

```

$ echo w > /proc/sysrq-trigger
$ dmesg -T


[Mon Nov  7 07:48:36 2022] task:khugepaged      state:D stack:    0 pid:   67 ppid:     2 flags:0x80004080
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  rwsem_down_write_slowpath+0x251/0x620
[Mon Nov  7 07:48:36 2022]  ? khugepaged+0xd44/0x2340
[Mon Nov  7 07:48:36 2022]  khugepaged+0xd44/0x2340
[Mon Nov  7 07:48:36 2022]  ? syscall_return_via_sysret+0x10/0x7f
[Mon Nov  7 07:48:36 2022]  ? syscall_return_via_sysret+0x10/0x7f
[Mon Nov  7 07:48:36 2022]  ? wait_woken+0x80/0x80
[Mon Nov  7 07:48:36 2022]  ? collapse_shmem+0xce0/0xce0
[Mon Nov  7 07:48:36 2022]  kthread+0x10d/0x130
[Mon Nov  7 07:48:36 2022]  ? kthread_park+0xa0/0xa0
[Mon Nov  7 07:48:36 2022]  ret_from_fork+0x35/0x40
[Mon Nov  7 07:48:36 2022] task:xfsaild/rbd3    state:D stack:    0 pid: 7620 ppid:     2 flags:0x80004000
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  xlog_wait_on_iclog+0x10d/0x130 [xfs]
[Mon Nov  7 07:48:36 2022]  ? wake_up_q+0xa0/0xa0
[Mon Nov  7 07:48:36 2022]  xfsaild+0x1dd/0x830 [xfs]
[Mon Nov  7 07:48:36 2022]  ? xfs_trans_ail_cursor_first+0x80/0x80 [xfs]
[Mon Nov  7 07:48:36 2022]  ? kthread+0x10d/0x130
[Mon Nov  7 07:48:36 2022]  kthread+0x10d/0x130
[Mon Nov  7 07:48:36 2022]  ? kthread_park+0xa0/0xa0
[Mon Nov  7 07:48:36 2022]  ret_from_fork+0x35/0x40
[Mon Nov  7 07:48:36 2022] task:vmstorage       state:D stack:    0 pid: 8912 ppid:  8541 flags:0x00000004
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  rwsem_down_read_slowpath+0x172/0x300
[Mon Nov  7 07:48:36 2022]  ? lookup_slow+0x27/0x50
[Mon Nov  7 07:48:36 2022]  lookup_slow+0x27/0x50
[Mon Nov  7 07:48:36 2022]  walk_component+0x1c4/0x300
[Mon Nov  7 07:48:36 2022]  ? link_path_walk.part.33+0x68/0x510
[Mon Nov  7 07:48:36 2022]  ? path_init+0x192/0x320
[Mon Nov  7 07:48:36 2022]  path_lookupat+0x6e/0x210
[Mon Nov  7 07:48:36 2022]  ? terminate_walk+0x8c/0x100
[Mon Nov  7 07:48:36 2022]  filename_lookup+0xb6/0x190
[Mon Nov  7 07:48:36 2022]  ? kmem_cache_alloc+0x18a/0x270
[Mon Nov  7 07:48:36 2022]  ? getname_flags+0x66/0x1d0
[Mon Nov  7 07:48:36 2022]  ? vfs_statx+0x73/0xe0
[Mon Nov  7 07:48:36 2022]  vfs_statx+0x73/0xe0
[Mon Nov  7 07:48:36 2022]  __do_sys_newfstatat+0x31/0x70
[Mon Nov  7 07:48:36 2022]  ? __do_sys_fstatfs+0x2f/0x50
[Mon Nov  7 07:48:36 2022]  ? _cond_resched+0x15/0x40
[Mon Nov  7 07:48:36 2022]  ? exit_to_usermode_loop+0xc5/0x120
[Mon Nov  7 07:48:36 2022]  do_syscall_64+0x5b/0x1e0
[Mon Nov  7 07:48:36 2022]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[Mon Nov  7 07:48:36 2022] RIP: 0033:0x4bcd2a
[Mon Nov  7 07:48:36 2022] Code: Bad RIP value.
[Mon Nov  7 07:48:36 2022] RSP: 002b:000000c00162d688 EFLAGS: 00000212 ORIG_RAX: 0000000000000106
[Mon Nov  7 07:48:36 2022] RAX: ffffffffffffffda RBX: 000000c000022000 RCX: 00000000004bcd2a
[Mon Nov  7 07:48:36 2022] RDX: 000000c000114518 RSI: 000000c012bfa500 RDI: ffffffffffffff9c
[Mon Nov  7 07:48:36 2022] RBP: 000000c00162d708 R08: 0000000000000000 R09: 0000000000000000
[Mon Nov  7 07:48:36 2022] R10: 0000000000000000 R11: 0000000000000212 R12: 0000000000000000
[Mon Nov  7 07:48:36 2022] R13: 0000000000000001 R14: 0000000000000015 R15: ffffffffffffffff
[Mon Nov  7 07:48:36 2022] task:vmstorage       state:D stack:    0 pid: 8945 ppid:  8541 flags:0x00000004
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  schedule_timeout+0x16e/0x2d0
[Mon Nov  7 07:48:36 2022]  ? netif_rx_internal+0x41/0x100
[Mon Nov  7 07:48:36 2022]  __down+0x91/0xe0
[Mon Nov  7 07:48:36 2022]  ? xfs_buf_find.isra.34+0x1e0/0x620 [xfs]
[Mon Nov  7 07:48:36 2022]  ? down+0x3b/0x50
[Mon Nov  7 07:48:36 2022]  down+0x3b/0x50
[Mon Nov  7 07:48:36 2022]  xfs_buf_lock+0x33/0xf0 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_buf_find.isra.34+0x1e0/0x620 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_buf_get_map+0x40/0x2a0 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_buf_read_map+0x28/0x1b0 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_trans_read_buf_map+0xc7/0x340 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_read_agi+0x95/0x140 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_ialloc_read_agi+0x2f/0xd0 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_dialloc+0x10e/0x280 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_ialloc+0x7c/0x530 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_dir_ialloc+0x69/0x200 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_create+0x3e1/0x5a0 [xfs]
[Mon Nov  7 07:48:36 2022]  xfs_gencom_create+0x106/0x340 [xfs]
[Mon Nov  7 07:48:36 2022]  vfs_mkdir+0x101/0x1b0
[Mon Nov  7 07:48:36 2022]  do_mkdirat+0xec/0x120
[Mon Nov  7 07:48:36 2022]  do_syscall_64+0x5b/0x1e0
[Mon Nov  7 07:48:36 2022]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[Mon Nov  7 07:48:36 2022] RIP: 0033:0x4bccbb
[Mon Nov  7 07:48:36 2022] Code: Bad RIP value.
[Mon Nov  7 07:48:36 2022] RSP: 002b:000000c00162b6b0 EFLAGS: 00000202 ORIG_RAX: 0000000000000102
[Mon Nov  7 07:48:36 2022] RAX: ffffffffffffffda RBX: 000000c000024800 RCX: 00000000004bccbb
[Mon Nov  7 07:48:36 2022] RDX: 00000000000001ed RSI: 000000c00d076d80 RDI: ffffffffffffff9c
[Mon Nov  7 07:48:36 2022] RBP: 000000c00162b710 R08: 0000000000000000 R09: 0000000000000000
[Mon Nov  7 07:48:36 2022] R10: 0000000000000000 R11: 0000000000000202 R12: 0000000000000000
[Mon Nov  7 07:48:36 2022] R13: 0000000000000001 R14: 0000000000000037 R15: ffffffffffffffff
...
[Mon Nov  7 07:48:36 2022] task:read-hotness-tr state:D stack:    0 pid: 7976 ppid:  5642 flags:0x00000024
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  io_schedule+0x12/0x40
[Mon Nov  7 07:48:36 2022]  wait_on_page_bit+0x13b/0x210
[Mon Nov  7 07:48:36 2022]  ? file_fdatawait_range+0x20/0x20
[Mon Nov  7 07:48:36 2022]  ? iomap_readpage_actor+0x350/0x350
[Mon Nov  7 07:48:36 2022]  iomap_page_mkwrite+0x107/0x150
[Mon Nov  7 07:48:36 2022]  __xfs_filemap_fault+0x13a/0x1f0 [xfs]
[Mon Nov  7 07:48:36 2022]  ? hrtimer_init_sleeper+0x90/0x90
[Mon Nov  7 07:48:36 2022]  do_page_mkwrite+0x3e/0x90
[Mon Nov  7 07:48:36 2022]  ? vm_normal_page+0x1a/0xb0
[Mon Nov  7 07:48:36 2022]  do_wp_page+0x26e/0x3b0
[Mon Nov  7 07:48:36 2022]  __handle_mm_fault+0xc98/0x1260
[Mon Nov  7 07:48:36 2022]  handle_mm_fault+0xc4/0x200
[Mon Nov  7 07:48:36 2022]  __do_page_fault+0x2ce/0x500
[Mon Nov  7 07:48:36 2022]  do_page_fault+0x30/0x110
[Mon Nov  7 07:48:36 2022]  page_fault+0x3e/0x50
[Mon Nov  7 07:48:36 2022] RIP: 0033:0x7f2df98b7625
[Mon Nov  7 07:48:36 2022] Code: Bad RIP value.
[Mon Nov  7 07:48:36 2022] RSP: 002b:00007f2dd6dfde10 EFLAGS: 00010246
[Mon Nov  7 07:48:36 2022] RAX: 0000000693ace8e0 RBX: 0000000693ace870 RCX: 00007f2db7559000
[Mon Nov  7 07:48:36 2022] RDX: 0000000072000000 RSI: 00000000f805a5f3 RDI: 00000000004a5661
[Mon Nov  7 07:48:36 2022] RBP: 0000000000000072 R08: 00000000004a5661 R09: 0000000693ace850
[Mon Nov  7 07:48:36 2022] R10: 0000000693ace8b0 R11: 00000000d2759d0e R12: 0000000000000000
[Mon Nov  7 07:48:36 2022] R13: 00000006a9197f78 R14: 00000000d5df4a69 R15: 00007f2de7d3d000
[Mon Nov  7 07:48:36 2022] task:kworker/u16:0   state:D stack:    0 pid:26317 ppid:     2 flags:0x80004000
[Mon Nov  7 07:48:36 2022] Workqueue: writeback wb_workfn (flush-254:16)
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  io_schedule+0x12/0x40
[Mon Nov  7 07:48:36 2022]  __lock_page+0x126/0x210
[Mon Nov  7 07:48:36 2022]  ? file_fdatawait_range+0x20/0x20
[Mon Nov  7 07:48:36 2022]  write_cache_pages+0x2c5/0x400
[Mon Nov  7 07:48:36 2022]  ? xfs_vm_writepages+0xa0/0xa0 [xfs]
[Mon Nov  7 07:48:36 2022]  ? update_load_avg+0x1ac/0x5f0
[Mon Nov  7 07:48:36 2022]  ? update_blocked_averages+0x297/0x520
[Mon Nov  7 07:48:36 2022]  xfs_vm_writepages+0x64/0xa0 [xfs]
[Mon Nov  7 07:48:36 2022]  do_writepages+0x4b/0xe0
[Mon Nov  7 07:48:36 2022]  ? __writeback_single_inode+0x39/0x300
[Mon Nov  7 07:48:36 2022]  __writeback_single_inode+0x39/0x300
[Mon Nov  7 07:48:36 2022]  writeback_sb_inodes+0x1ad/0x4b0
[Mon Nov  7 07:48:36 2022]  __writeback_inodes_wb+0x5d/0xb0
[Mon Nov  7 07:48:36 2022]  wb_writeback+0x25e/0x2f0
[Mon Nov  7 07:48:36 2022]  ? wb_workfn+0x23c/0x4e0
[Mon Nov  7 07:48:36 2022]  wb_workfn+0x23c/0x4e0
[Mon Nov  7 07:48:36 2022]  ? finish_task_switch+0x7e/0x2a0
[Mon Nov  7 07:48:36 2022]  ? process_one_work+0x1f4/0x3e0
[Mon Nov  7 07:48:36 2022]  ? inode_wait_for_writeback+0x30/0x30
[Mon Nov  7 07:48:36 2022]  process_one_work+0x1f4/0x3e0
[Mon Nov  7 07:48:36 2022]  worker_thread+0x2d/0x3e0
[Mon Nov  7 07:48:36 2022]  ? process_one_work+0x3e0/0x3e0
[Mon Nov  7 07:48:36 2022]  kthread+0x10d/0x130
[Mon Nov  7 07:48:36 2022]  ? kthread_park+0xa0/0xa0
[Mon Nov  7 07:48:36 2022]  ret_from_fork+0x35/0x40
[Mon Nov  7 07:48:36 2022] task:kworker/u16:1   state:D stack:    0 pid:12572 ppid:     2 flags:0x80004000
[Mon Nov  7 07:48:36 2022] Workqueue: writeback wb_workfn (flush-254:0)
[Mon Nov  7 07:48:36 2022] Call Trace:
[Mon Nov  7 07:48:36 2022]  __schedule+0x2ff/0x760
[Mon Nov  7 07:48:36 2022]  schedule+0x2f/0xa0
[Mon Nov  7 07:48:36 2022]  io_schedule+0x12/0x40
[Mon Nov  7 07:48:36 2022]  __lock_page+0x126/0x210
[Mon Nov  7 07:48:36 2022]  ? file_fdatawait_range+0x20/0x20
[Mon Nov  7 07:48:36 2022]  write_cache_pages+0x2c5/0x400
[Mon Nov  7 07:48:36 2022]  ? xfs_vm_writepages+0xa0/0xa0 [xfs]
[Mon Nov  7 07:48:36 2022]  ? blk_mq_dispatch_rq_list+0x2ae/0x670
[Mon Nov  7 07:48:36 2022]  ? bfq_dispatch_request+0x211/0xfa0
[Mon Nov  7 07:48:36 2022]  xfs_vm_writepages+0x64/0xa0 [xfs]
[Mon Nov  7 07:48:36 2022]  do_writepages+0x4b/0xe0
[Mon Nov  7 07:48:36 2022]  ? __blk_mq_sched_dispatch_requests+0x151/0x170
[Mon Nov  7 07:48:36 2022]  ? __writeback_single_inode+0x39/0x300
[Mon Nov  7 07:48:36 2022]  __writeback_single_inode+0x39/0x300
[Mon Nov  7 07:48:36 2022]  writeback_sb_inodes+0x1ad/0x4b0
[Mon Nov  7 07:48:36 2022]  __writeback_inodes_wb+0x5d/0xb0
[Mon Nov  7 07:48:36 2022]  wb_writeback+0x25e/0x2f0
[Mon Nov  7 07:48:36 2022]  ? wb_workfn+0x23c/0x4e0
[Mon Nov  7 07:48:36 2022]  wb_workfn+0x23c/0x4e0
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x34/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x34/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x40/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x34/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x40/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x34/0x70
[Mon Nov  7 07:48:36 2022]  ? __switch_to_asm+0x40/0x70
[Mon Nov  7 07:48:36 2022]  ? process_one_work+0x1f4/0x3e0
[Mon Nov  7 07:48:36 2022]  ? inode_wait_for_writeback+0x30/0x30
[Mon Nov  7 07:48:36 2022]  process_one_work+0x1f4/0x3e0
[Mon Nov  7 07:48:36 2022]  worker_thread+0x2d/0x3e0
[Mon Nov  7 07:48:36 2022]  ? process_one_work+0x3e0/0x3e0
[Mon Nov  7 07:48:36 2022]  kthread+0x10d/0x130
[Mon Nov  7 07:48:36 2022]  ? kthread_park+0xa0/0xa0
[Mon Nov  7 07:48:36 2022]  ret_from_fork+0x35/0x40
```



## 参考
 - {doc}`/kernel/namespace/pid-namespace/trouble-cases/PID1-in-containers`
 - [https://www.geekersdigest.com/the-end-of-the-kill-road-the-uninterruptable-sleep-state/](https://www.geekersdigest.com/the-end-of-the-kill-road-the-uninterruptable-sleep-state/)

