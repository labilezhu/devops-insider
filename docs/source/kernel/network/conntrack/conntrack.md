# conntrack

## Concept

> - [连接跟踪（conntrack）：原理、应用及 Linux 内核实现 @ arthurchiao.art/blog](https://arthurchiao.art/blog/conntrack-design-and-implementation-zh/#5-%E9%85%8D%E7%BD%AE%E5%92%8C%E7%9B%91%E6%8E%A7), [English ver](https://arthurchiao.art/blog/conntrack-design-and-implementation/)
> - [http://43.129.181.206/share/linux2.6-kernel-summary.pdf](http://43.129.181.206/share/linux2.6-kernel-summary.pdf) {download}`Download Linux2.6 内核总结（庞宇） </kernel/network/conntrack/linux2.6-kernel-summary.pdf>` .


```{toctree}
ref/connection_tracking_1_modules_and_hooks.md
ref/connection_tracking_2_modules_and_hooks.md
ref/connection_tracking_3_modules_and_hooks.md
ref/conntrack-design-and-implementation-zh.md
```



## Monitoring & Troubleshooting
> - [https://conntrack-tools.netfilter.org/manual.html](https://conntrack-tools.netfilter.org/manual.html)
> - [https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/](https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/)
> - [https://fedoramagazine.org/network-address-translation-part-2-the-conntrack-tool/](https://fedoramagazine.org/network-address-translation-part-2-the-conntrack-tool/)
> - [Oracle Linux: IPTABLES conntrack Table Gets Stuck with an Entry in SYN_SENT State](https://support.oracle.com/knowledge/Oracle%20Linux%20and%20Virtualization/2870462_1.html)


```{toctree}
:maxdepth: 2
conntrack-troubleshooting/network-address-translation-part-4-conntrack-troubleshooting.md
conntrack-troubleshooting/out-of-window-invalid-conntrack.md
conntrack-troubleshooting/local-port-collision-syn-retransmit.md
conntrack-troubleshooting/iptables-troubleshooting.md
```



### real-time conntrack event log

> [https://fedoramagazine.org/conntrack-event-framework/](https://fedoramagazine.org/conntrack-event-framework/)

```
# conntrack -E
NEW tcp     120 SYN_SENT src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 [UNREPLIED] src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123
UPDATE tcp      60 SYN_RECV src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123
UPDATE tcp  432000 ESTABLISHED src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123 [ASSURED]
UPDATE tcp     120 FIN_WAIT src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123 [ASSURED]
UPDATE tcp      30 LAST_ACK src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123 [ASSURED]
UPDATE tcp     120 TIME_WAIT src=10.1.0.114 dst=10.7.43.52 sport=4123 dport=22 src=10.7.43.52 dst=10.1.0.114 sport=22 dport=4123 [ASSURED]
```


## Tuning

- [Netfilter Conntrack Sysfs variables@kernel.org](https://www.kernel.org/doc/html/latest/networking/nf_conntrack-sysctl.html)
- [/proc/sys/net/netfilter/nf_conntrack_* Variables](https://www.kernel.org/doc/Documentation/networking/nf_conntrack-sysctl.txt)

- [https://blog.cloudflare.com/conntrack-tales-one-thousand-and-one-flows/](https://blog.cloudflare.com/conntrack-tales-one-thousand-and-one-flows/)

### iptables NAT random local port

- [iptables --random and --random-fully ignore /proc/sys/net/ipv4/ip_local_port_range #509](https://github.com/aws/amazon-vpc-cni-k8s/issues/509)
- [Packets getting lost during SNAT with too many connections](https://opendev.org/openstack/neutron/commit/ce628a123769f93fc0c1b2edbe20ec5325aab0f6)



## Ref.

```{toctree}
ref/conntrack-tales@cloudflare.md
conntrack-tcp-reset.md
```
