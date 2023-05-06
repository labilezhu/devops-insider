# Check MTU

## tracepath


> https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/network_mtu.html



```bash
[ec2-user ~]$ tracepath amazon.com
 1?: [LOCALHOST]     pmtu 9001
 1:  ip-172-31-16-1.us-west-1.compute.internal (172.31.16.1)   0.187ms pmtu 1500
 1:  no reply
 2:  no reply
 3:  no reply
 4:  100.64.16.241 (100.64.16.241)                          0.574ms
 5:  72.21.222.221 (72.21.222.221)                         84.447ms asymm 21
 6:  205.251.229.97 (205.251.229.97)                       79.970ms asymm 19
 7:  72.21.222.194 (72.21.222.194)                         96.546ms asymm 16
 8:  72.21.222.239 (72.21.222.239)                         79.244ms asymm 15
 9:  205.251.225.73 (205.251.225.73)                       91.867ms asymm 16
...
31:  no reply
     Too many hops: pmtu 1500
     Resume: pmtu 1500
```


## Linux route cache - ip route get

> https://jasonmurray.org/posts/2020/linuxmtucache/

```log
[jemurray@wuit-s-00047 ~]$ sudo ip route get 104.131.191.87
104.131.191.87 via 128.252.5.114 dev p1p1 src 128.252.5.113
    cache
```

Sending a packet using ICMP > 1500 bytes, note the ICMP Frag needed (refer to the packet capture below for more details):

```log
[jemurray@wuit-s-00047 ~]$ ping -M do -s 1500 shell.jasonmurray.org
PING jasonmurray.org (104.18.61.146) 1500(1528) bytes of data.
From xe-0-0-1-900w-mmr-wu-rt-0.net.wustl.edu (128.252.100.126) icmp_seq=1 Frag needed and DF set (mtu = 1500)
ping: local error: Message too long, mtu=1500
ping: local error: Message too long, mtu=1500
ping: local error: Message too long, mtu=1500
```

Linux caches the 1500 byte maximum MTU path length:
```log
[jemurray@wuit-s-00047 ~]$ sudo ip route get 104.131.191.87
104.131.191.87 via 128.252.5.114 dev p1p1 src 128.252.5.113
    cache expires 579sec mtu 1500
```

### Routing Cache 中包含什么信息
> http://linux-ip.net/html/routing-cache.html

The following attributes may be stored for each entry in the routing cache.

- cwnd, FIXME Window
  FIXME. A) I don't know what it is. B) I don't know how to describe it.

- advmss, Advertised Maximum Segment Size

- src, (Preferred Local) Source Address

- mtu, Maximum Transmission Unit

- rtt, Round Trip Time

- rttvar, Round Trip Time Variation

  FIXME. Gotta find some references to this, too.

- age
- users
- used

