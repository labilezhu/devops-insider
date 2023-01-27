# iptables Troubleshooting

## iptables stat

> [https://unix.stackexchange.com/questions/384880/how-to-get-metrics-about-dropped-traffic-via-iptables](https://unix.stackexchange.com/questions/384880/how-to-get-metrics-about-dropped-traffic-via-iptables)



```
$ sudo iptables -L -n -v -x
Chain INPUT (policy DROP 0 packets, 0 bytes)
 pkts bytes target prot opt in out source    destination 
   39 22221 ACCEPT udp  --  *  *   0.0.0.0/0  0.0.0.0/0 udp spts:67:68 dpts:67:68
 ...
  182 43862 LOG    all  --  *  *   0.0.0.0/0  0.0.0.0/0 LOG flags 0 level 4 prefix "input_drop: "
  182 43862 REJECT all  --  *  *   0.0.0.0/0  0.0.0.0/0 reject-with icmp-host-prohibited
```

shows no dropped packets on my local network but 182 rejected with icmp and a log message such as the one you listed. The last two rules in the configuration with a policy of DROP were

```
  -A INPUT -j LOG --log-prefix "input_drop: "
  -A INPUT -j REJECT --reject-with icmp-host-prohibited
```

You can zero the counters for all chains with `iptables -Z`.



> - [Istio- Improve iptables debugability #33965](https://github.com/istio/istio/issues/33965)

- Iptables allows inserting comments (`--comment`) which can help explain why we have certain rules.
- Iptables allows inserting logs (`iptables -A INPUT -j LOG`, for example). This could be helpful to debug the flow of traffic. I don't think you can add a log a specific line, but for every rule we create we can make a log statement with the same match I think.



Like need `echo 1 | sudo tee /proc/sys/net/netfilter/nf_log_all_netns` to get logs in containers

