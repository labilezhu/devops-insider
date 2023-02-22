---
title: "tracepoint - Linux 内核跟踪点"
date: 2021-03-08T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- trace
- kernel
- tracepoint
---


## List tracepoints

### by perf

```
root@worknode5:/home/labile# perf list tracepoint

List of pre-defined events (to be used in -e):

  tcp:tcp_destroy_sock                               [Tracepoint event]
  tcp:tcp_probe                                      [Tracepoint event]
  tcp:tcp_rcv_space_adjust                           [Tracepoint event]
  tcp:tcp_receive_reset                              [Tracepoint event]
  tcp:tcp_retransmit_skb                             [Tracepoint event]
  tcp:tcp_retransmit_synack                          [Tracepoint event]
  tcp:tcp_send_reset                                 [Tracepoint event]


Metric Groups:

  sock:inet_sock_set_state                           [Tracepoint event]


Metric Groups:

```



### By BCC tplist
```
sudo tplist-bpfcc -v 'tcp:*' 
```

### bpftrace
```
bpftrace -l 't:sock:*'
```

### By kernel filesystem

```
root@worknode5:/home/labile# sudo ls   /sys/kernel/debug/tracing/events
alarmtimer    btrfs   compaction  devlink    exceptions  filelock  ftrace        huge_memory  initcall     irq          kmem    mce      module  neigh  page_isolation  power   ras           regulator  rtc     skb    swiotlb     tcp                      tlb       wbt        xdp
block         cgroup  cpuhp       dma_fence  ext4        filemap   gpio          hwmon        intel_iommu  irq_matrix   kvm     mdio     mpx     net    page_pool       printk  raw_syscalls  resctrl    sched   smbus  sync_trace  thermal                  udp       workqueue  xen
bpf_test_run  clk     cros_ec     drm        fib         fs        header_event  hyperv       iocost       irq_vectors  kvmmmu  migrate  msr     nmi    pagemap         qdisc   rcu           rpm        scsi    sock   syscalls    thermal_power_allocator  vmscan    writeback  xhci-hcd
bridge        cma     devfreq     enable     fib6        fs_dax    header_page   i2c          iommu        jbd2         libata  mmc      napi    oom    percpu          random  regmap        rseq       signal  spi    task        timer                    vsyscall  x86_fpu


root@worknode5:/home/labile# sudo ls   /sys/kernel/debug/tracing/events/tcp/
enable  filter  tcp_destroy_sock  tcp_probe  tcp_rcv_space_adjust  tcp_receive_reset  tcp_retransmit_skb  tcp_retransmit_synack  tcp_send_reset


root@worknode5:/home/labile# sudo cat  /sys/kernel/debug/tracing/events/tcp/tcp_retransmit_skb/format 
name: tcp_retransmit_skb
ID: 1400
format:
        field:unsigned short common_type;       offset:0;       size:2; signed:0;
        field:unsigned char common_flags;       offset:2;       size:1; signed:0;
        field:unsigned char common_preempt_count;       offset:3;       size:1; signed:0;
        field:int common_pid;   offset:4;       size:4; signed:1;

        field:const void * skbaddr;     offset:8;       size:8; signed:0;
        field:const void * skaddr;      offset:16;      size:8; signed:0;
        field:int state;        offset:24;      size:4; signed:1;
        field:__u16 sport;      offset:28;      size:2; signed:0;
        field:__u16 dport;      offset:30;      size:2; signed:0;
        field:__u8 saddr[4];    offset:32;      size:4; signed:0;
        field:__u8 daddr[4];    offset:36;      size:4; signed:0;
        field:__u8 saddr_v6[16];        offset:40;      size:16;        signed:0;
        field:__u8 daddr_v6[16];        offset:56;      size:16;        signed:0;

print fmt: "sport=%hu dport=%hu saddr=%pI4 daddr=%pI4 saddrv6=%pI6c daddrv6=%pI6c state=%s", REC->sport, REC->dport, REC->saddr, REC->daddr, REC->saddr_v6, REC->daddr_v6, __print_symbolic(REC->state, { TCP_ESTABLISHED, "TCP_ESTABLISHED" }, { TCP_SYN_SENT, "TCP_SYN_SENT" }, { TCP_SYN_RECV, "TCP_SYN_RECV" }, { TCP_FIN_WAIT1, "TCP_FIN_WAIT1" }, { TCP_FIN_WAIT2, "TCP_FIN_WAIT2" }, { TCP_TIME_WAIT, "TCP_TIME_WAIT" }, { TCP_CLOSE, "TCP_CLOSE" }, { TCP_CLOSE_WAIT, "TCP_CLOSE_WAIT" }, { TCP_LAST_ACK, "TCP_LAST_ACK" }, { TCP_LISTEN, "TCP_LISTEN" }, { TCP_CLOSING, "TCP_CLOSING" }, { TCP_NEW_SYN_RECV, "TCP_NEW_SYN_RECV" })



```

## Tracepoint definition
> https://blogs.oracle.com/linux/post/taming-tracepoints-in-the-linux-kernel

​    

Tracepoints are defined in header files under *include/trace/events*.

Our *netif_rx* tracepoint is defined in *include/trace/events/net.h*.

Each tracepoint definition consists of a description of

- TP_PROTO, the function prototype used in calling the tracepoint. In the case of netif_rx(), that's simply *struct sk_buff \*skb*
- TP_ARGS, the argument names; (*skb*)
- TP_STRUCT__entry field definitions; these correspond to the fields which are assigned when the tracepoint is triggered
- TP_fast_assign statements which take the raw argument to the tracepoint (the skb) and set the associated field values (skb len, skb pointer etc)
- TP_printk which is responsible for using those field values to display a relevant tracing message

Note: tracepoints defined via TRACE_EVENT specify all of the above, whereas we can also define an event class which shares fields, assignments and messages. In fact *netif_rx* is of event class *net_dev_template*, so our field assignments and message come from that event class.

## Tracepoint arguments and format string
```bash
cat /sys/kernel/debug/tracing/events/block/block_rq_issue/format

cat /sys/kernel/debug/tracing/events/syscalls/sys_enter_accept4/format

name: sys_enter_accept4
ID: 1369
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int __syscall_nr;	offset:8;	size:4;	signed:1;
	field:int fd;	offset:16;	size:8;	signed:0;
	field:struct sockaddr * upeer_sockaddr;	offset:24;	size:8;	signed:0;
	field:int * upeer_addrlen;	offset:32;	size:8;	signed:0;
	field:int flags;	offset:40;	size:8;	signed:0;

print fmt: "fd: 0x%08lx, upeer_sockaddr: 0x%08lx, upeer_addrlen: 0x%08lx, flags: 0x%08lx", ((unsigned long)(REC->fd)), ((unsigned long)(REC->upeer_sockaddr)), ((unsigned long)(REC->upeer_addrlen)), ((unsigned long)(REC->flags))

cat /sys/kernel/debug/tracing/events/syscalls/sys_enter_connect/format
name: sys_enter_connect
ID: 1365
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int __syscall_nr;	offset:8;	size:4;	signed:1;
	field:int fd;	offset:16;	size:8;	signed:0;
	field:struct sockaddr * uservaddr;	offset:24;	size:8;	signed:0;
	field:int addrlen;	offset:32;	size:8;	signed:0;
print fmt: "fd: 0x%08lx, uservaddr: 0x%08lx, addrlen: 0x%08lx", ((unsigned long)(REC->fd)), ((unsigned long)(REC->uservaddr)), ((unsigned long)(REC->addrlen))


cat /sys/kernel/debug/tracing/events/syscalls/sys_enter_bind/format 
name: sys_enter_bind
ID: 1373
format:
        field:unsigned short common_type;       offset:0;       size:2; signed:0;
        field:unsigned char common_flags;       offset:2;       size:1; signed:0;
        field:unsigned char common_preempt_count;       offset:3;       size:1; signed:0;
        field:int common_pid;   offset:4;       size:4; signed:1;

        field:int __syscall_nr; offset:8;       size:4; signed:1;
        field:int fd;   offset:16;      size:8; signed:0;
        field:struct sockaddr * umyaddr;        offset:24;      size:8; signed:0;
        field:int addrlen;      offset:32;      size:8; signed:0;

print fmt: "fd: 0x%08lx, umyaddr: 0x%08lx, addrlen: 0x%08lx", ((unsigned long)(REC->fd)), ((unsigned long)(REC->umyaddr)), ((unsigned long)(REC->addrlen))



```

## Tracepoint interface
* Kernel File System: /sys/kernel/debug/tracing
* perf_event_open(2) syscall

## Tracepoint for TCP

> https://www.brendangregg.com/blog/2018-03-22/tcp-tracepoints.html

During development I talked to Song (and Alexei Starovoitov) about the recent additions, so I already have an idea about why these exist and their use. Some rough notes for the current TCP tracepoints:

- **tcp:tcp_retransmit_skb**: Traces retransmits. Useful for understanding network issues including congestion. Will be used by my tcpretrans tools instead of kprobes.
- **tcp:tcp_retransmit_synack**: Tracing SYN/ACK retransmits. I imagine this could be used for a type of DoS detection (SYN flood triggering SYN/ACKs and then retransmits). This is separate from tcp:tcp_retransmit_skb because this event doesn't have an skb.
- **tcp:tcp_destroy_sock**: Needed by any program that summarizes details in-memory for a TCP session, which would be keyed by the sock address. This probe can be used to know that the session has ended, so that sock address is about to be reused and any summarized data so far should be consumed and then deleted.
- **tcp:tcp_send_reset**: This traces a RST send during a valid socket, to diagnose those type of issues.
- **tcp:tcp_receive_reset**: Trace a RST receive.
- **tcp:tcp_probe**: for TCP congestion window tracing, which also allowed an older TCP probe module to be deprecated and removed. This was [added by](https://lkml.org/lkml/2017/12/19/154) Masami Hiramatsu, and merged in Linux 4.16.
- **sock:inet_sock_set_state**: Can be used for many things. The tcplife tool is one, but also my tcpconnect and tcpaccept bcc tools can be converted to use this tracepoint. We could add separate `tcp:tcp_connect` and `tcp:tcp_accept` tracepoints (or `tcp:tcp_active_open` and `tcp:tcp_passive_open`), but `sock:inet_sock_set_state` can be used for this.


## Ref.
* https://www.kernel.org/doc/Documentation/trace/tracepoints.txt
* https://blogs.oracle.com/linux/taming-tracepoints-in-the-linux-kernel
* http://www.brendangregg.com/blog/2018-03-22/tcp-tracepoints.html
* [BPF Performance Tools]
* https://blogs.oracle.com/linux/post/taming-tracepoints-in-the-linux-kernel