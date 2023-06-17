---
title: "ipvs load balancer"
date: 2021-05-08T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- cloud
- network
---

## IPVS as load balancer
> https://medium.com/@benmeier_/a-quick-minimal-ipvs-load-balancer-demo-d5cc42d0deb4

#### Get stats

```
$ sudo ipvsadm -L -n --stats --rate
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port    Conns   InPkts  OutPkts  InBytes OutBytes
  -> RemoteAddress:Port
TCP  1.2.3.4:80           80      560     400      36000   41040
  -> 172.17.0.3:80        22      154      110     9900    11286
  -> 172.17.0.4:80        58      406      290     26100   29754
```



## Ref.

* https://medium.com/@benmeier_/a-quick-minimal-ipvs-load-balancer-demo-d5cc42d0deb4
* https://debugged.it/blog/ipvs-the-linux-load-balancer/
* https://kubernetes.io/blog/2018/07/09/ipvs-based-in-cluster-load-balancing-deep-dive/