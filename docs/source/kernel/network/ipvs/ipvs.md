# IPVS

## Setting

### IPVS timeout

> [http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/LVS-HOWTO.ipvsadm.html](http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/LVS-HOWTO.ipvsadm.html)
>
> There's a lot of TCP timers in the Linux kernel and they all have different semantical meanings. There is the TCP timout timer for sockets related to locally initiated connections, then there is a TCP timeout for the connection tracking table, which on my desktop system for example has following settings:
>
> ```
> /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_close:10 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_close_wait:60 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established:432000 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_fin_wait:120 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_last_ack:30 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_syn_recv:60 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_syn_sent:120 /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_time_wait:120 
> ```
>
> And of course we have the IPVS TCP settings, which look as follows (if they weren't disabled in the core :)):
>
> ```
> /proc/sys/net/ipv4/vs/tcp_timeout_established:900 /proc/sys/net/ipv4/vs/tcp_timeout_syn_sent:120 /proc/sys/net/ipv4/vs/tcp_timeout_syn_recv:60 /proc/sys/net/ipv4/vs/tcp_timeout_:900 [...] 
> ```
>
> unless you enabled tcp_defense, which changes those timers again. And then of course we have other in-kernel timers, which influence those timers mentioned above.
>
> However, the beforementioned timers regarding packet filtering, NAPT and load balancing and are meant as a means to map expected real TCP flow timeouts. Since there is no socket (as in an endpoint) involved when doing either netfilter or IPVS, you have to guess what the TCP flow in-between (where you machine is "standing") is doing, so you can continue to forward, rewrite, mangle, whatever, the flow, _without_ disturbing it. The timers are used for table mapping timeouts of TCP states. If we didn't have them, mappings would stay in the kernel forever and eventually we'd run out of memory. If we have them wrong, it might occur that a connection is aborted prematurely by our host, for example yielding those infamous ssh hangs when connecting through a packet filter.



```bash
$ ipvsadm -l --timeout
Timeout (tcp tcpfin udp): 900 120 300

$ ipvsadm --set 3600 120 300
```

#### IPVS timeout 与 tcp keepalive 的关系

//TBD

```
tcp_keepalive_time = 7200 (seconds)
tcp_keepalive_intvl = 75 (seconds)
tcp_keepalive_probes = 9 (number of probes)
```





### Monitoring







## Ref.

 - [http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/index.html](http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/index.html)
 - [http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/LVS-HOWTO.LVS-NAT.html](http://www.austintek.com/LVS/LVS-HOWTO/HOWTO/LVS-HOWTO.LVS-NAT.html)
 - [LVS Offical Documentation](http://www.linuxvirtualserver.org/Documents.html)

