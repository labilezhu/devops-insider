# 定位 conntrack TCP RESET 问题

## How connection tracking works at a high level

> [https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/](https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/)



The connection tracker first extracts the IP addresses and higher-level protocol information from the packet. “Higher level protocol information” is the transport protocol specific part. A common example are the source and destination port numbers (tcp, udp) or the ICMP id. A more exotic example would be the PPTP call id. These packet fields – the IP addresses and protocol specific information – are the lookup keys used to check the connection tracking table.

In addition to checking if a packet is new or part of a known connection, conntrack also performs protocol specific tests. In case of UDP, it checks if the packet is complete (received packet length matches length specified in the UDP header) and that the UDP checksum is correct. For other protocols, such as TCP, it will also check:

- Are TCP flags valid (for example a packet is considered invalid if both RST and SYN flags are set)
- When a packet acknowledges data, it checks that the acknowledgment number matches data sent in the other direction.
- When a packet contains new data, <mark>it checks that this data is within the receive window announced by the peer</mark>.

Any failures in these checks cause the packet to be considered **invalid**. For such packets, conntrack will neither create a new connection tracking entry nor associate it with an existing entry, even if one exists. Conntrack can be configured to log a reason for why a packet was deemed to be invalid:

```
# sysctl -a | grep nf_log
net.netfilter.nf_log.10 = NONE
net.netfilter.nf_log.2 = NONE
net.netfilter.nf_log_all_netns = 0
```



The following examples occur with the settings created using *“sudo sysctl net.netfilter.nf_log.2=nf_log_ipv4”* and “*sudo sysctl net.netfilter.nf_conntrack_log_invalid=6*“.

```
nf_ct_proto_6: invalid packet ignored in state ESTABLISHED SRC=10.47.217.34 DST=192.168.0.17 LEN=60 DF SPT=443 DPT=47832 SEQ=389276 ACK=3130 WINDOW=65160 ACK SYN
```



## Out of TCP window retrans

*  [kube-proxy Subtleties: Debugging an Intermittent Connection Reset](https://kubernetes.io/blog/2019/03/29/kube-proxy-subtleties-debugging-an-intermittent-connection-reset/)

![Connection reset packet flow](conntrack-tcp-reset.assets/connection-reset-packet-flow.png)

* [Add workaround for spurious retransmits leading to connection resets #1090](https://github.com/moby/libnetwork/issues/1090)

> Here is a summary of what happens:
>
> 1. For some reason, when an AWS EC2 machine connects to itself using its external-facing IP address, there are occasional packets with sequence numbers and timestamps that are far behind the rest.
> 2. Normally these packets would be ignored as spurious retransmits. However, because the packets fall outside the TCP window, Linux's conntrack module marks them invalid, and their destination addresses do not get rewritten by DNAT.
> 3. The packets are eventually interpreted as packets destined to the actual address/port in the IP/TCP headers. Since there is no flow matching these, the host sends a RST.
> 4. The RST terminates the actual NAT'd connection, since its source address and port matches the NAT'd connection.





## Runing Out of max conntrack table size



-  https://deploy.live/blog/kubernetes-networking-problems-due-to-the-conntrack/



## Ref

> - [https://netfilter-devel.vger.kernel.narkive.com/cjiYhM93/tcp-window-tracking-has-bad-side-effects](https://netfilter-devel.vger.kernel.narkive.com/cjiYhM93/tcp-window-tracking-has-bad-side-effects)
>
> 



