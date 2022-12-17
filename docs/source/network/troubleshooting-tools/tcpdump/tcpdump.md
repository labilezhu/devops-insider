# tcpdump

## 指定 ip:port 组合

```bash
tcpdump -i lo -n -vvvv -A  "((dst port 22 and dst 127.0.0.1) or (src port 22 and src 127.0.0.1))"

# 思考，和这个一样吗： tcpdump -i lo 'host 127.0.0.1 and port 22'
```

## 指定 port

```bash
# 对外的连接 tcp port
tcpdump -i eth0 -n -vvvv -A  "((src portrange 20000-65535 and src $ETH0_IP) or (dst portrange 20000-65535 and dst $ETH0_IP))"
```

## 根据 RST FIN SYN 过滤

```bash
# Show all RST packets:
tcpdump -i eth0 'tcp[13] & 4 != 0'
tcpdump -i eth0 "tcp[tcpflags] & (tcp-rst) != 0"


# Show all FIN packets:
tcpdump -i eth0 'tcp[13] & 1 != 0'
tcpdump -i eth0 "tcp[tcpflags] & (tcp-fin) != 0"

# Show all SYN packets:
tcpdump -i eth0 'tcp[13] & 2 != 0'
tcpdump -i eth0 "tcp[tcpflags] & (tcp-syn) != 0"

# Show all SYN/FIN/RST packets:
tcpdump -i eth0 "tcp[tcpflags] & (tcp-syn|tcp-fin|tcp-rst) != 0"
```


```bash
export MY_IP=119.142.139.39 #update it

$ip route get $MY_IP
#119.142.139.39 via 119.63.71.129 dev eth3 src 119.63.71.132 uid 1001
#    cache 
export INTERFACE=eth3 #update it by above output

sudo tcpdump -i $INTERFACE -c 9999 -w /tmp/tcpdump.pcap  "host $MY_IP and (tcp[13] & 1 != 0 or tcp[13] & 4 != 0 or tcp[13] & 2 != 0) " #only capture RST or FIN or SYN

```


## subnet 过滤

可以用 subnet/CIDR 过滤数据包。使用 CIDR 时需要注意，有时 copy 过来的 CIDR 不一定是规范的格式，如：

首先，看看 kubernetes cluster 的 pod 的 cidr 范围：

```bash
ps -ef | grep cidr
root      48587  20177  0 Dec08 ?        00:21:25 kube-controller-manager ... --cluster-cidr=192.168.0.0/12 ...--service-cluster-ip-range=10.96.0.0/12 ...
```



这时，如果尝试直接使用上面的参数会出错：

```bash
$ sudo tcpdump -i br0 -vvvv -A  net 192.168.0.0/12
tcpdump: non-network bits set in "192.168.0.0/12"
```



发现，tcpdump 对 cidr 的格式要求比较严格，要求用首个可用 ip 段。使用 [https://cidr.xyz/](https://cidr.xyz/) 分析出 `192.168.0.0/12` 的首个可用 ip 为 `192.160.0.1`，固：

```bash
sudo tcpdump -i br0 -vvvv -A  net 192.160.0.0/12
```

## 抓取的 packet 大小限制

By default most newer implementations of tcpdump will capture 65535 bytes, however in some situations you may not want to capture the default packet length. You can use -s to specify the “snaplen” or “snapshot length” that you want tcpdump to capture.


```
       -s snaplen
       --snapshot-length=snaplen
              Snarf snaplen bytes of data from each packet rather than the default of 262144 bytes.  Packets truncated because of a limited snapshot are indicated in the output with ``[|proto]'', where proto is the name of the protocol level at  which  the  truncation
              has occurred.

              Note  that  taking  larger  snapshots both increases the amount of time it takes to process packets and, effectively, decreases the amount of packet buffering.  This may cause packets to be lost.  Note also that taking smaller snapshots will discard data
              from protocols above the transport layer, which loses information that may be important.  NFS and AFS requests and replies, for example, are very large, and much of the detail won't be available if a too-short snapshot length is selected.

              If you need to reduce the snapshot size below the default, you should limit snaplen to the smallest number that will capture the protocol information you're interested in.  Setting snaplen to 0 sets it to the default of 262144, for backwards  compatibility with recent older versions of tcpdump.
```

> [https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html](https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html)
> 
Older versions of tcpdump truncate packets to 68 or 96 bytes. If this is the case, use -s to capture full-sized packets:

```bash
$ tcpdump -i <interface> -s 65535 -w <file>
```

https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html

## 注意事项

### 抓包 位置与 iptable/netfilter/conntrack 的关系

tcpdump 的抓包点。因为，这个点影响了，tcpdump 的过滤条件与输出。主要是这个点与 `iptables` 的 `redirect` 规则生效的前后问题。

> tcpdump capture pinpoint:
>
> tcpdump will see inbound traffic before iptables, but will see
> outbound traffic only after the firewall has processed it.
>
> As a matter of fact, tcpdump is the first software found after the wire (and the NIC, if you will) on the way IN, and the last one on the way OUT.
>
> * Wire -> NIC -> tcpdump -> netfilter/iptables
> * iptables -> tcpdump -> NIC -> Wire



## 杂项技巧


### 本地 IP 变量
```
export ETH0_IP=$(ip addr show eth0 | grep "inet\b" | awk '{print $2}' | cut -d/ -f1)
export LOCAL_IP=$(ip addr show lo | grep "inet\b" | awk '{print $2}' | cut -d/ -f1)
```




## Ref.

 - [A tcpdump Tutorial with Examples — 50 Ways to Isolate Traffic](https://danielmiessler.com/study/tcpdump/)
 - [Istio 端口 与 组件 - 抓包](https://istio-insider.mygraphql.com/zh_CN/latest/ch1-istio-arch/istio-ports-components.html#id5)