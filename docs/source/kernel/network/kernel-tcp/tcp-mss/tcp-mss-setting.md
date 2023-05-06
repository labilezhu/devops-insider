# TCP MSS Setting

## TCP MSS(advmss) Setting

###  MSS Option

> [TCP IP Architecture Design - Sameer Seth]


Maximum segment size (mss) is a mere refl ection of maximum size of the TCP   payload that can be accepted by the remote host. mss is a function of the maximum   transmission unit (MTU), which is a property of the link layer. So, TCP has to work   in coordination with the IP layer to arrive at this value. It is the IP layer which fi nds   out the lowest MTU for the internet path (MTU discovery, RFC 1191). RFC 793   specifi es that standards to arrive at the send and receive mss for TCP. **The `mss` option is always exchanged with the TCP SYN segment at the time of connection initialization**. The idea of exchanging mss information is to improve the performance of TCP.   In the case where sending TCP can send more than the receiving end can accept,   the IP datagram will be fragmented at the IP layer. Each fragment is now transmitted with the header overhead consuming the bandwidth. If any of the fragment is   not received or lost, the entire TCP segment needs to be retransmitted hitting the   throughput. On the other hand, if the sender TCP is generating smaller TCP seg-   ments with default mss (536 bytes) where it is capable of sending bigger segments   and the other end is also capable of receiving bigger TCP segments, TCP will be   operating at lower throughput and hence low performance. Format for the mss   option is shown in Fig. 2.4 .


### Customize SYN MSS Option
> https://medium.com/@knownsec404team/analysis-of-linux-kernel-tcp-mss-mechanism-e9560bd312f6#:~:text=Customize%20SYN%20MSS%20Option

There are three ways to set the MSS value of the TCP SYN packet.

1. iptable

```bash
# Add rules  
$ sudo iptables -I OUTPUT -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 48  
# delete rules  
$ sudo iptables -D OUTPUT -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 48
```

2. ip route advmss

```bash
# show router information  
$ route -ne  
$ ip route show  
192.168.11.0/24 dev ens33 proto kernel scope link src 192.168.11.111 metric 100  
# modify route table  
$ sudo ip route change 192.168.11.0/24 dev ens33 proto kernel scope link src 192.168.11.111 metric 100 advmss 48
```

```
advmss NUMBER (Linux 2.3.15+ only)

the MSS ('Maximal Segment Size') to advertise to these destinations when establishing TCP connections. If it is not given, Linux uses a default value calculated from the first hop device MTU. (If the path to these destination is asymmetric, this guess may be wrong.)
```

Document:
https://linux.die.net/man/8/ip#:~:text=Slow%20Start%20value.-,advmss,-NUMBER%20(2.3.15%2B%20only


## 指定出向报文的 MSS 限制

### Disable Path MTU discovery for specify ip/subnet by setting it manually

> [https://tldp.org/HOWTO/Adv-Routing-HOWTO/lartc.cookbook.mtu-discovery.html](https://tldp.org/HOWTO/Adv-Routing-HOWTO/lartc.cookbook.mtu-discovery.html)

```bash
ip route add default via 10.0.0.1 mtu 296
```

In general, it is possible to override PMTU Discovery by setting specific routes. For example, if only a certain subnet is giving problems, this should help:

```bash
ip route add 195.96.96.0/24 via 10.0.0.1 mtu 1000
```

### 加锁定 - ip route add mtu lock
> https://netdev.vger.kernel.narkive.com/JBda7zob/per-route-mtu-settings


```bash
ip route add 10.81.105.109/32 via 10.81.104.1 mtu lock 1450
ip route flush cache
```

> https://www.systutorials.com/docs/linux/man/8-ip-route/#:~:text=mtu%20MTU-,mtu%20lock%20MTU,-the%20MTU%20along
> - mtu `lock` MTU
the MTU along the path to the destination. If the modifier `lock` is not used, the MTU may be updated by the kernel due to Path MTU Discovery. If the modifier `lock` is used, no path MTU discovery will be tried, all packets will be sent without the DF bit in IPv4 case or fragmented to MTU for IPv6.
