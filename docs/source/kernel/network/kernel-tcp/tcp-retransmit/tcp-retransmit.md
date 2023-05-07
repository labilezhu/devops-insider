# tcp retransmit

## Setting

```
proc/sys/net/ipv4/tcp_syn_retries
proc/sys/net/ipv4/tcp_synack_retries
proc/sys/net/ipv4/tcp_retries1
proc/sys/net/ipv4/tcp_retries2
proc/sys/net/ipv4/tcp_early_retrans
proc/sys/net/ipv4/tcp_retrans_collapse
proc/sys/net/ipv4/tcp_orphan_retries
```

## initial RTO

- [Customize TCP initial RTO (retransmission timeout) with BPF](https://arthurchiao.art/blog/customize-tcp-initial-rto-with-bpf/)
- [BPF: The future of configs - Set default retransmit time](https://blog.habets.se/2020/11/BPF-the-future-of-configs.html#:~:text=Set%20default%20retransmit%20time)


## RTO

- [聊一聊重传次数 - perthcharles.github.io](https://perthcharles.github.io/2015/09/07/wiki-tcp-retries/)


## Ref
 - [TCP Retransmission May Be Misleading - arthurchiao.art](http://arthurchiao.art/blog/tcp-retransmission-may-be-misleading/)
 - [TCP的MTU Probe、MSS、Retransmit 的关系 - cloud.tencent.com](https://cloud.tencent.com/developer/beta/article/1411873)
