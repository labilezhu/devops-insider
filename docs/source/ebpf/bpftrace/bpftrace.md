---
title: "系统级跟踪 eBPF 工具 —— bpftrace 入门"
date: 2021-03-08T15:12:15+09:00
draft: false
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- trace
- kernel
- ebpf
---

## bpftrace 简介

## bpftrace 简单使用

#### 查询可以跟踪的内核函数，以 sleep 为关键字

```bash
$ bpftrace -l '*open*'

tracepoint:syscalls:sys_exit_open_tree
tracepoint:syscalls:sys_enter_open
...
kprobe:vfs_open
kprobe:tcp_try_fastopen
...

```

#### 跟踪所有 sys_enter_open() 系统调用

```bash
$ bpftrace -e 'tracepoint:syscalls:sys_enter_open{ printf("%s %s\n", comm,str(args->filename)); }' | grep vi
```

然后在另外一个终端中

```bash
$ vi /etc/hosts
```

可以看到在 bpftrace 终端中输出：

```
vi /usr/lib/locale/locale-archive
vi /usr/share/locale/locale.alias
vi /usr/lib/locale/zh_CN.UTF-8/LC_IDENTIFICATION
...
^C
```

#### 跟踪所有 sys_enter_open*()，输出总计数

```bash
$ bpftrace -e 'tracepoint:syscalls:sys_enter_open*{ @[probe] = count(); }'
Attaching 4 probes...
^C

@[tracepoint:syscalls:sys_enter_openat]: 123
@[tracepoint:syscalls:sys_enter_open]: 628
```



## 参考

[BPF Performance Tools]