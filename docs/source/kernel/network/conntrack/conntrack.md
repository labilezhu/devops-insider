# conntrack

## Concept

> 连接跟踪（conntrack）：原理、应用及 Linux 内核实现 : https://arthurchiao.art/blog/conntrack-design-and-implementation-zh/#5-%E9%85%8D%E7%BD%AE%E5%92%8C%E7%9B%91%E6%8E%A7(https://arthurchiao.art/blog/conntrack-design-and-implementation-zh/#5-%E9%85%8D%E7%BD%AE%E5%92%8C%E7%9B%91%E6%8E%A7)

> https://tech.xing.com/a-reason-for-unexplained-connection-timeouts-on-kubernetes-docker-abd041cf7e02(https://tech.xing.com/a-reason-for-unexplained-connection-timeouts-on-kubernetes-docker-abd041cf7e02)


```{toctree}
ref/connection_tracking_1_modules_and_hooks.md
ref/connection_tracking_2_modules_and_hooks.md
ref/connection_tracking_3_modules_and_hooks.md
```

## Monitoring & Troubleshooting
> [https://conntrack-tools.netfilter.org/manual.html](https://conntrack-tools.netfilter.org/manual.html)
> [https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/](https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/)
> [https://fedoramagazine.org/network-address-translation-part-2-the-conntrack-tool/](https://fedoramagazine.org/network-address-translation-part-2-the-conntrack-tool/)


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

```{toctree}
:maxdepth: 2
conntrack-troubleshooting/network-address-translation-part-4-conntrack-troubleshooting.md
```


## Tuning

* Netfilter Conntrack Sysfs variables : https://www.kernel.org/doc/html/latest/networking/nf_conntrack-sysctl.html

> https://blog.cloudflare.com/conntrack-tales-one-thousand-and-one-flows/


## Ref.
```{toctree}
ref/conntrack-tales@cloudflare.md
conntrack-tcp-reset.md
```
