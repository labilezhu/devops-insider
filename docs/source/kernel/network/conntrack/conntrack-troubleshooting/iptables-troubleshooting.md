# iptables Troubleshooting

## iptables LOG target options

> [LOG target options@iptables tutorial](https://www.frozentux.net/iptables-tutorial/iptables-tutorial.html#LOGTARGET:~:text=2.5%20and%202.6.-,LOG%20target%20options,-The%20LOG%20target)


The LOG target is specially designed for logging detailed information about packets. These could, for example, be considered as illegal. Or, logging can be used purely for bug hunting and error finding. The LOG target will return specific information on packets, such as most of the IP headers and other information considered interesting. It does this via the `kernel logging facility`, normally `syslogd`. This information may then be read directly with `dmesg`, or from the `syslogd logs`, or with other programs or applications. This is an excellent target to use to debug your rule-sets, so that you can see what packets go where and what rules are applied on what packets. Note as well that it could be a really great idea to use the LOG target instead of the DROP target while you are testing a rule you are not 100% sure about on a production firewall, since a syntax error in the rule-sets could otherwise cause severe connectivity problems for your users. Also note that the ULOG target may be interesting if you are using really extensive logging, since the ULOG target has support for direct logging to MySQL databases and suchlike.

>  Note that if you get undesired logging direct to consoles, this is not an iptables or Netfilter problem, but rather a problem caused by your syslogd configuration - most probably `/etc/syslog.conf`. Read more in man syslog.conf for information about this kind of problem.You may also need to tweak your dmesg settings. dmesg is the command that changes which errors from the kernel that should be shown on the console. dmesg -n 1 should prevent all messages from showing up on the console, except panic messages. The dmesg message levels matches exactly the syslogd levels, and it only works on log messages from the kernel facility. For more information, see man dmesg.



The LOG target currently takes five options that could be of interest if you have specific information needs, or want to set different options to specific values. They are all listed below.



**Table 11-8. LOG target options**

|             |                                                              |
| ----------- | ------------------------------------------------------------ |
| **Option**  | **--log-level**                                              |
| Example     | iptables -A FORWARD -p tcp -j LOG --log-level debug          |
| Explanation | This is the option to tell iptables and syslog which log level to use. For a complete list of log levels read the `syslog.conf` manual. Normally there are the following log levels, or priorities as they are normally referred to: <br />debug, info, notice, warning, warn, err, error, crit, alert, emerg and panic. <br />The keyword `error` is the same as `err`, `warn` is the same as `warning` and `panic` is the same as `emerg`. **Note that all three of these are deprecated, in other words do not use error, warn and panic**. The priority defines the severity of the message being logged. All messages are logged through the kernel facility. In other words, setting `kern.=info /var/log/iptables` in your `syslog.conf` file and then letting all your LOG messages in iptables use log level info, would make all messages appear in the `/var/log/iptables` file. Note that there may be other messages here as well from other parts of the kernel that uses the info priority. For more information on logging I recommend you to read the syslog and `syslog.conf` man-pages as well as other HOWTOs etc. |
| **Option**  | **--log-prefix**                                             |
| Example     | iptables -A INPUT -p tcp -j LOG --log-prefix "INPUT packets" |
| Explanation | This option tells iptables to prefix all log messages with a specific prefix, which can then easily be combined with grep or other tools to track specific problems and output from different rules. **The prefix may be up to 29 letters long**, including white-spaces and other special symbols. |
| **Option**  | **--log-tcp-sequence**                                       |
| Example     | iptables -A INPUT -p tcp -j LOG --log-tcp-sequence           |
| Explanation | This option will log the TCP Sequence numbers, together with the log message. The TCP Sequence numbers are special numbers that identify each packet and where it fits into a TCP sequence, as well as how the stream should be reassembled. Note that this option constitutes a security risk if the logs are readable by unauthorized users, or by the world for that matter. As does any log that contains output from iptables. |
| **Option**  | **--log-tcp-options**                                        |
| Example     | iptables -A FORWARD -p tcp -j LOG --log-tcp-options          |
| Explanation | The --log-tcp-options option logs the different options from the TCP packet headers and can be valuable when trying to debug what could go wrong, or what has actually gone wrong. This option does not take any variable fields or anything like that, just as most of the LOG options don't. |
| **Option**  | **--log-ip-options**                                         |
| Example     | iptables -A FORWARD -p tcp -j LOG --log-ip-options           |
| Explanation | The --log-ip-options option will log most of the IP packet header options. This works exactly the same as the --log-tcp-options option, but instead works on the IP options. These logging messages may be valuable when trying to debug or track specific culprits, as well as for debugging - in just the same way as the previous option. |



更多例子：

[https://www.netfilter.org/documentation/FAQ/netfilter-faq-3.html](https://www.netfilter.org/documentation/FAQ/netfilter-faq-3.html)



- How do I stop the LOG target from logging to my console?

You have to configure your syslogd and/or klogd appropriately: The LOG target logs to facility kern at priority warning (4). See the syslogd.conf manpage to learn more about facilities and priorities.

By default, all kernel messages at priority more severe than debug (7) are sent to the console. If you raise that to 4, instead of 7, you will make the LOG messages no longer appear on the console.

Be aware that this might also suppress other important messages from appearing on the console (does not affect syslog).



Istio 相关使用：

- [Improve iptables debugability#34189](https://github.com/istio/istio/pull/34189)
- https://istio.io/latest/docs/reference/commands/pilot-agent/#:~:text=IPTABLES_TRACE_LOGGING



## **iptables stat**

> **[https://unix.stackexchange.com/questions/384880/how-to-get-metrics-about-dropped-traffic-via-iptables](https://unix.stackexchange.com/questions/384880/how-to-get-metrics-about-dropped-traffic-via-iptables)**



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



Like need `echo 1 | sudo tee /proc/sys/net/netfilter/nf_log_all_netns` to get logs in containers(其实应该不需要，见正文中这个开关的使用场景)



## **为什么kill进程后socket一直处于FIN_WAIT_1状态**

> **[为什么kill进程后socket一直处于FIN_WAIT_1状态](https://developer.aliyun.com/article/704262)**

问题的现象：ECS上有一个进程，建立了到另一个服务器的socket连接。 kill掉进程，发现tcpdump抓不到FIN包发出，导致服务器端的连接没有正常关闭。

利用iptables -nvL可以打出每条rule匹配到的计数，或者利用写log的办法，示例如下：

```bash
# 记录下new state的报文的日志
iptables -A INPUT -p tcp -m state --state NEW -j LOG --log-prefix "[iptables] INPUT NEW: "
```

在这个案例中，通过计数和近一步的log，发现了是OUTPUT chain的最后一跳DROP规则被匹配上了，如下：

```
# iptables -A OUTPUT -m state --state INVALID -j DROP
```



### FIN包被认为是INVALID状态？

对于一个TCP连接，在conntrack中没有连接跟踪表项，一端FIN掉连接的时候的时候被认为是INVALID状态是很符合逻辑的事情。但是没有发现任何文档清楚地描述这个场景：当用户空间TCP socket仍然存在，但是conntrack表项已经不存在时，对一个“新”的报文，conntrack模块认为它是什么状态。

所有文档描述conntrack的NEW, ESTABLISHED, RELATED, INVALID状态时大同小异，比较详细的描述如[文档](https://www.frozentux.net/iptables-tutorial/chunkyhtml/x1358.html)：

> The NEW state tells us that the packet is the first packet that we see. This means that the first packet that the conntrack module sees, within a specific connection, will be matched. For example, if we see a SYN packet and it is the first packet in a connection that we see, it will match. However, the packet may as well not be a SYN packet and still be considered NEW. This may lead to certain problems in some instances, but it may also be extremely helpful when we need to pick up lost connections from other firewalls, or when a connection has already timed out, but in reality is not closed.

如上对于NEW状态的描述为：conntrack module看见的一个报文就是NEW状态，例如TCP的SYN报文，有时候非SYN也被认为是NEW状态。

在本案例的场景里，conntrack表项已经过期了，此时不管从用户态发什么报文到conntrack模块时，都算是conntrack模块看见的第一个报文，那么conntrack都认为是NEW状态吗？比如SYN, SYNACK, FIN, RST，这些明显有不同的语义，实践经验FIN, RST这些直接放成INVALID是没毛病的。



例子：

```bash
#!/bin/sh
iptables -P INPUT ACCEPT
iptables -F
iptables -X
iptables -Z
# 在日志里记录INPUT chain里过来的每个报文的状态
iptables -A INPUT -p tcp -m state --state NEW -j LOG --log-prefix "[iptables] INPUT NEW: "
iptables -A INPUT -p TCP -m state --state ESTABLISHED -j LOG --log-prefix "[iptables] INPUT ESTABLISHED: "
iptables -A INPUT -p TCP -m state --state RELATED -j LOG --log-prefix "[iptables] INPUT RELATED: "
iptables -A INPUT -p TCP -m state --state INVALID -j LOG --log-prefix "[iptables] INPUT INVALID: "
iptables -A INPUT -i lo -j ACCEPT
iptables -A INPUT -p tcp --dport 22 -j ACCEPT
iptables -A INPUT -p tcp --dport 21 -j ACCEPT
iptables -A INPUT -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -p tcp --dport 443 -j ACCEPT
iptables -A INPUT -p tcp --dport 8088 -m state --state NEW -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 8 -j ACCEPT
iptables -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
# 在日志里记录OUTPUT chain里过来的每个报文的状态
iptables -A OUTPUT -p tcp -m state --state NEW -j LOG --log-prefix "[iptables] OUTPUT NEW: "
iptables -A OUTPUT -p TCP -m state --state ESTABLISHED -j LOG --log-prefix "[iptables] OUTPUT ESTABLISHED: "
iptables -A OUTPUT -p TCP -m state --state RELATED -j LOG --log-prefix "[iptables] OUTPUT RELATED: "
iptables -A OUTPUT -p TCP -m state --state INVALID -j LOG --log-prefix "[iptables] OUTPUT INVALID: "
# iptables -A OUTPUT -m state --state INVALID -j DROP
iptables -P INPUT DROP
iptables -P OUTPUT ACCEPT
iptables -P FORWARD DROP
service iptables save
systemctl restart iptables.service
```





## **Log internal conntrack information**

> **[https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/](https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/)**

The “_net.netfilter.nf\_conntrack\_log\_invalid″_ sysctl is used to set kernel parameters to get more information about why a packet is considered invalid. The default setting, 0, disables this logging. Positive numbers (up to 255) specify for which protocol more information will be logged. For example, _6_ would print more information for tcp, while 17 would provide more information for udp. The numbers are identical to those found in the file _/etc/protocols._ The special value _255_ enables debug logging for all protocol trackers.

You may need to set a specific logging backend. Use “_sysctl -a | grep nf\_log_” to see what log backends are currently in use. NONE means that no backend is set. Example output:

```bash
# sysctl -a | grep nf_log
net.netfilter.nf_log.10 = NONE
net.netfilter.nf_log.2 = NONE
net.netfilter.nf_log_all_netns = 0
```



2 is ipv4, 3 is arp, 7 is used for bridge logging and 10 for ipv6. For connection tracking only ipv4 (2) and ipv6 (10) are relevant. The last sysctl shown here – _nf\_log\_all\_netns_ – is set to the default 0 to prevent other namespaces from flooding the system log. It may be set to 1 to debug issues in another network namespace.

### nf_log_all_netns

> https://www.kernel.org/doc/Documentation/networking/netfilter-sysctl.txt
>
> /proc/sys/net/netfilter/* Variables:
>
> nf_log_all_netns - BOOLEAN
> 	0 - disabled (default)
> 	not 0 - enabled
>
> 	By default, only init_net namespace can log packets into kernel log
> 	with LOG target; this aims to prevent containers from flooding host
> 	kernel log. If enabled, this target also works in other network
> 	namespaces. This variable is only accessible from init_net.



> https://serverfault.com/questions/691730/iptables-log-rule-inside-a-network-namespace
>
> As Donald mentioned, iptables LOG rules inside containers are suppressed by default.
>
> In kernels <=4.10, this behavior could not be adjusted without patching the kernel. As agrrd mentioned, a work-around is to run ulogd in each container and use iptables NFLOG (or ULOG) rules instead of LOG rules.
>
> However, as of kernel 4.11, running `echo 1 > /proc/sys/net/netfilter/nf_log_all_netns` on the host (outside of the container) will cause iptables LOG rules inside all containers to log to the host. (See this Kernel Commit.)



Ref:

- https://www.linkedin.com/pulse/how-take-iptables-trace-inside-container-syed-miftahur-rahman?trk=articles_directory
- https://bookstack.swigg.net/books/linux/page/netfilteriptable-logging
- https://github.com/moby/moby/issues/10540#issuecomment-260422783
- https://patchwork.ozlabs.org/project/netfilter-devel/patch/20160428074838.0BBB5A0C94@unicorn.suse.cz/







### Logger configuration

This command will print a list of available log modules:

```
# ls /lib/modules/$(uname -r)/kernel/net/netfilter/log /lib/modules/$(uname -r)/kernel/net/ip/netfilter/log* /lib/modules/$(uname -r)/kernel/net/ipv4/netfilter/nf_log*
```



The command:

```
modinfo nf_log_ipv4
# modprobe nf_log_ipv4
```



loads the ipv4 log module. If multiple log modules are loaded you can set the preferred/active logger with sysctl. For example:

```
# sudo sysctl net.netfilter.nf_log.2=nf_log_ipv4
```



tells the kernel to log ipv4 packet events to syslog/journald. This only affects log messages generated by conntrack debugging. Log messages generated by rules like “`ipables -j NFLOG`” or the _LOG_ target do not change as the rule itself already specifies to log type to use (nfnetlink and syslog/journald respectively).

After this, debug messages will appear in ulogd (if configured via nfnetlink) or the system log (if nf\_log\_ipv4 is the log backend).

### Example debug output

The following examples occur with the settings created using “`sudo sysctl net.netfilter.nf_log.2=nf_log_ipv4`” and 

`sudo sysctl net.netfilter.nf_conntrack_log_invalid=6`.

```
 nf_ct_proto_6: invalid packet ignored in state ESTABLISHED SRC=10.47.217.34 DST=192.168.0.17 LEN=60 DF SPT=443 DPT=47832 SEQ=389276 ACK=3130 WINDOW=65160 ACK SYN

nf_ct_proto_6: ACK is over the upper bound (ACKed data not seen yet) SRC=10.3.1.1 DST=192.168.0.1 LEN=60 DF SPT=443 DPT=49135 SEQ= ...
```



This dump contains the packet contents (allowing correlation with tcpdump packet capture of the flow, for example) plus a reason why the packet was tagged as INVALID.

### Dynamic Printk

If further information is needed, there are log statements in the conntrack module that can be enabled at run-time with the dynamic debugging infrastructure.

To check if this feature is available, use the following command:

```
# sudo grep nf_conntrack_proto_tcp /sys/kernel/debug/dynamic_debug/control
```



If the conntrack module is loaded and the dynamic debug feature is available, the output is similar to this:

```
net/netfilter/nf_conntrack_proto_tcp.c:1104 [nf_conntrack]nf_conntrack_tcp_packet =_ "syn=%i ack=%i fin=%i rst=%i old=%i new=%i\012"

net/netfilter/nf_conntrack_proto_tcp.c:1102 [nf_conntrack]nf_conntrack_tcp_packet =_ "tcp_conntracks: " net/netfilter/nf_conntrack_proto_tcp.c:1005 [nf_conntrack]nf_conntrack_tcp_packet =_ "nf_ct_tcp: Invalid dir=%i index=%u ostate=%u\012"

net/netfilter/nf_conntrack_proto_tcp.c:999 [nf_conntrack]nf_conntrack_tcp_packet =_ "nf_ct_tcp: SYN proxy client keep alive\012"
```



Ref. https://access.redhat.com/solutions/5069611



Each line shows the location of a default-disabled debug _printk_ statement. _printk_ is a C function from the Linux kernel interface that prints messages to the kernel log. The name of the file in the linux kernel source code comes first, followed by the line number. The square brackets contain the name of the kernel module that this source file is part of. The combination of file name and line number allows enabling or disabling these _printk_ statements. This command:

```
# sudo echo "file net/netfilter/nf_conntrack_proto_tcp.c line 1005 +p" &gt; /sys/kernel/debug/dynamic_debug/control
```



will enable the _printk_ statement shown in line 1005 of [net/netfilter/nf\_conntrack\_proto\_tcp.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/net/netfilter/nf_conntrack_proto_tcp.c?id=e2ef5203c817a60bfb591343ffd851b6537370ff#n1005). The same command, with “_+p_” replaced by “_\-p_“, disables this log line again. This facility is not unique to connection tracking: many parts of the kernel provide such debug messages. This technique is useful when things go wrong and more information about the conntrack internal state is needed. A dedicated howto about the dynamic debug feature is available in the kernel documentation [here](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/dynamic-debug-howto.rst?id=e85d92b3bc3b7062f18b24092a65ec427afa8148).

### The unconfirmed and dying lists

A newly allocated conntrack entry is first added to the unconfirmed list. Once the packet is accepted by all iptables/nftables rules, the conntrack entry moves from the unconfirmed list to the main connection tracking table. The dying list is the inverse: when a entry is removed from the table, it is placed on the dying list. The entry is freed once all packets that reference the flow have been dropped. This means that a conntrack entry is always on a list: Either the unconfirmed list, the dying list, or the conntrack hash table list. Most entries will be in the hash table.

If removal from the table is due to a timeout, no further references exist and the entry is freed immediately. This is what will typically happen with UDP flows. For TCP, conntrack entries are normally removed due to a special TCP packets such as the last TCP acknowledgment or a TCP reset. This is because TCP, unlike UDP, signals state transitions, such as connection closure. The entry is moved from the table to the dying list. The conntrack entry is then released after the network stack has processed the “last packet” packet.

#### Examining these lists

```bash
# sudo conntrack -L unconfirmed
# sudo conntrack -L dying
```



These two commands show the lists. A large discrepancy between the number of active connections (_sudo conntrack -C_) and the content of the connection tracking table (_sudo conntrack -L_) indicate a problem. Entries that remain on either one of these lists for long time periods indicate a kernel bug. Expected time ranges are in the order of a few microseconds.



