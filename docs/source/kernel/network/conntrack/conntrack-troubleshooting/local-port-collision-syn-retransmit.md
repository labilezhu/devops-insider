# NAT local port collision and SYN retransmit (NAT 分配端口冲突与 SYN 重传)

NAT 分配端口冲突导致连接慢与 SYN 重传。

> [A reason for unexplained connection timeouts on Kubernetes/Docker](https://tech.new-work.se/a-reason-for-unexplained-connection-timeouts-on-kubernetes-docker-abd041cf7e02)

The Linux Kernel has a known race condition when doing source network address translation (SNAT) that can lead to SYN packets being dropped. SNAT is performed by default on outgoing connections with [Docker](https://www.docker.com/) and [Flannel](https://github.com/coreos/flannel) using iptables masquerading rules. The race can happen when multiple containers try to establish new connections to the same external address concurrently. In some cases, two connections can be allocated the same port for the translation which ultimately results in one or more packets being dropped and at least one second connection delay.

This race condition is [mentioned in the source code](https://github.com/torvalds/linux/blob/24de3d377539e384621c5b8f8f8d8d01852dddc8/net/netfilter/nf_nat_core.c#L290-L291) but there is not much documentation around it. While the Kernel already supports a flag that mitigates this issue, it was not supported on iptables masquerading rules until recently.

In this post we will try to explain how we investigated that issue, what this race condition consists of with some explanations about container networking, and how we mitigated it.

**Edit 15/06/2018**: the same race condition exists on DNAT. On Kubernetes, this means you can lose packets when reaching ClusterIPs. For those who don’t know about DNAT, it’s probably best to read this article first but basically, when you do a request from a Pod to a ClusterIP, by default kube-proxy (through iptables) changes the ClusterIP with one of the PodIP of the service you are trying to reach. One of the most used cluster Service is the DNS and this race condition would generate intermitent delays when doing name resolution, see [issue 56903](https://github.com/kubernetes/kubernetes/issues/56903) or this [interesting article from Quentin Machu](https://blog.quentin-machu.fr/2018/06/24/5-15s-dns-lookups-on-kubernetes/).

**Edit 16/05/2021**: more detailed instructions to reproduce the issue have been added to https://github.com/maxlaverse/snat-race-conn-test



## Conntrack in user-space

We had the strong assumption that having most of our connections always going to the same `host:port` could be the reason why we had those issues. However, at this point we thought the problem could be caused by some misconfigured SYN flood protection. We read the description of network Kernel parameters hoping to discover some mechanism we were not aware of. We could not find anything related to our issue. We had already increased the size of the conntrack table and the Kernel logs were not showing any errors.

The second thing that came into our minds was port reuse. If we reached port exhaustion and there were no ports available for a SNAT operation, the packet would probably be dropped or rejected. We decided to look at the conntrack table. This also didn’t help very much as the table was underused but we discovered that the conntrack package had a command to display some statistics (`conntrack -S`). There was one field that immediately got our attention when running that command: “`insert_failed`” with a non-zero value.



## Netfilter NAT & Conntrack kernel modules

After reading the kernel netfilter code, we decided to recompile it and add some traces to get a better understanding of what was really happening. Here is what we learned.

The NAT code is hooked twice on the `POSTROUTING` chain ([1](http://inai.de/images/nf-packet-flow.png)). First to modify the packet structure by changing the source IP and/or PORT ([2](https://github.com/torvalds/linux/blob/1c8c5a9d38f607c0b6fd12c91cbe1a4418762a21/net/ipv4/netfilter/nf_nat_l3proto_ipv4.c#L358-L364)) and then to record the transformation in the conntrack table if the packet was not dropped in-between ([4](https://github.com/torvalds/linux/blob/24de3d377539e384621c5b8f8f8d8d01852dddc8/net/ipv4/netfilter/nf_conntrack_l3proto_ipv4.c#L196-L202)). This means there is a delay between the SNAT port allocation and the insertion in the table that might end up with an insertion failure if there is a conflict, and a packet drop. This is precisely what we see.

When doing SNAT on a tcp connection, the NAT module tries following ([5](https://github.com/torvalds/linux/blob/24de3d377539e384621c5b8f8f8d8d01852dddc8/net/netfilter/nf_nat_core.c#L290-L301)):

1. if the source IP of the packet is in the targeted NAT pool and the tuple is available then return (packet is kept unchanged).
2. find the least used IPs of the pool and replace the source IP in the packet with it
3. check if the port is in the allowed port range (default `1024-64512`) and if the tuple with that port is available. If that's the case, return (source IP was changed, port was kept). *(note: the SNAT port range is not influenced by the value of the* `*net.ipv4.ip_local_port_range*`*kernel parameters)*
4. the port is not available so ask the tcp layer to find a unique port for SNAT by calling `nf_nat_l4proto_unique_tuple()` ([3](https://github.com/torvalds/linux/blob/24de3d377539e384621c5b8f8f8d8d01852dddc8/net/netfilter/nf_nat_proto_common.c#L37-L85)).

When a host runs only one container, the NAT module will most probably return after the third step. The local port used by the process inside the container will be preserved and used for the outgoing connection. When running multiple containers on a Docker host, it is more likely that the source port of a connection is already used by the connection of another container. . In that case, `nf_nat_l4proto_unique_tuple()` is called to find an available port for the NAT operation.

The default port allocation does following:

1. copy the last allocated port from a shared value. This value is used a starting offset for the search
2. increment it by one
3. check if the port is used by calling `nf_nat_used_tuple()` and start over from 2. if that's the case
4. update the shared value of the last allocated port and return

Since there is a delay between the port allocation and the insertion of the connection in the conntrack table, `nf_nat_used_tuple()` can return true for a same port multiple times. And because `nf_nat_l4proto_unique_tuple()` can be called in parallel, the allocation sometimes starts with the same initial port value. On our test setup, most of the port allocation conflicts happened if the connections were initialized in the same 0 to 2us. Those values depend on a lot a different factors but give an idea of the timing order of magnitude.

netfilter also supports two other algorithms to find free ports for SNAT:

- using some randomness when settings the port allocation search offset. This mode is used when the SNAT rule has a flag `NF_NAT_RANGE_PROTO_RANDOM` active.
- using full randomness with the flag `NF_NAT_RANGE_PROTO_RANDOM_FULLY`. This takes a random number for the search offset.

`NF_NAT_RANGE_PROTO_RANDOM` lowered the number of times two threads were starting with the same initial port offset but there were still a lot of errors. It's only with `NF_NAT_RANGE_PROTO_RANDOM_FULLY` that we managed to reduce the number of insertion errors significantly. On a Docker test virtual machine with default masquerading rules and 10 to 80 threads making connection to the same host, we had from 2% to 4% of insertion failure in the conntrack table.

With full randomness forced in the Kernel, the errors dropped to 0 (and later near to 0 on live clusters).

## Activating full random port allocation on Kubernetes

The `NF_NAT_RANGE_PROTO_RANDOM_FULLY` flag needs to be set on masquerading rules. On our Kubernetes setup, Flannel is responsible for adding those rules. It uses iptables which it builds from the source code during the Docker image build. The iptables tool doesn't support setting this flag but we've committed a small [patch](https://git.netfilter.org/iptables/commit/?id=8b0da2130b8af3890ef20afb2305f11224bb39ec) that was merged (not released) and adds this feature.

We now use a modified version of Flannel that applies this patch and adds the `--random-fully` flag on the masquerading rules (4 lines change). The conntrack statistics are fetched on each node by a small DaemonSet, and the metrics sent to InfluxDB to keep an eye on insertion errors. We have been using this patch for a month now and the number of errors dropped from one every few seconds for a node, to one error every few hours on the whole clusters.





## Ref

> - [https://github.com/jwkohnen/conntrack-stats-exporter](https://github.com/jwkohnen/conntrack-stats-exporter)
> - https://blog.quentin-machu.fr/2018/06/24/5-15s-dns-lookups-on-kubernetes/
> - https://github.com/kubernetes/kubernetes/issues/56903
> - 
>
> 
