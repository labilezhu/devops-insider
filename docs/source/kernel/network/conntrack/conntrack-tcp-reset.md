# 定位 conntrack TCP RESET 问题

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



- https://deploy.live/blog/kubernetes-networking-problems-due-to-the-conntrack/
- 





