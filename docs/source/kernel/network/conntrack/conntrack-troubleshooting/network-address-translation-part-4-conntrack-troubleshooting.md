# Network address translation part 4 – Conntrack troubleshooting

> [https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/](https://fedoramagazine.org/network-address-translation-part-4-conntrack-troubleshooting/)



## Connection tracking and NAT

NAT configured via iptables or nftables builds on top of the netfilters connection tracking facility. This means that if there is any problem with the connection tracking engine, NAT will not work. This may result in connectivity issues. Ineffective NAT rules will leak internal addresses to the outer network. Use the nftables “*ct state*” or the iptables “*-m conntrack –ctstate*” feature to prevent this. If a packet matches the INVALID state, conntrack failed to associate the packet with a known connection. This also means NAT will not work.

## How connection tracking works at a high level

The connection tracker first extracts the IP addresses and higher-level protocol information from the packet. “Higher level protocol information” is the transport protocol specific part. A common example are the source and destination port numbers (tcp, udp) or the ICMP id. A more exotic example would be the PPTP call id. These packet fields – the IP addresses and protocol specific information – are the lookup keys used to check the connection tracking table.

In addition to checking if a packet is new or part of a known connection, conntrack also performs protocol specific tests. In case of UDP, it checks if the packet is complete (received packet length matches length specified in the UDP header) and that the UDP checksum is correct. For other protocols, such as TCP, it will also check:

- Are TCP flags valid (for example a packet is considered invalid if both RST and SYN flags are set)
- When a packet acknowledges data, it checks that the acknowledgment number matches data sent in the other direction.
- When a packet contains new data, it checks that this data is within the receive window announced by the peer.

Any failures in these checks cause the packet to be considered invalid. For such packets, conntrack will neither create a new connection tracking entry nor associate it with an existing entry, even if one exists. Conntrack can be configured to log a reason for why a packet was deemed to be invalid.

## Log internal conntrack information

The “*net.netfilter.nf_conntrack_log_invalid″* sysctl is used to set kernel parameters to get more information about why a packet is considered invalid. The default setting, 0, disables this logging. Positive numbers (up to 255) specify for which protocol more information will be logged. For example, *6* would print more information for tcp, while 17 would provide more information for udp. The numbers are identical to those found in the file */etc/protocols.* The special value *255* enables debug logging for all protocol trackers.

You may need to set a specific logging backend. Use “*sysctl -a | grep nf_log*” to see what log backends are currently in use. NONE means that no backend is set. Example output:

```
# sysctl -a | grep nf_log
net.netfilter.nf_log.10 = NONE
net.netfilter.nf_log.2 = NONE
net.netfilter.nf_log_all_netns = 0
```

2 is ipv4, 3 is arp, 7 is used for bridge logging and 10 for ipv6. For connection tracking only ipv4 (2) and ipv6 (10) are relevant. The last sysctl shown here – *nf_log_all_netns* – is set to the default 0 to prevent other namespaces from flooding the system log. It may be set to 1 to debug issues in another network namespace.

## Logger configuration

This command will print a list of available log modules:

```
# ls /lib/modules/$(uname -r)/kernel/net/netfilter/log /lib/modules/$(uname -r)/kernel/net/ip/netfilter/log*
```

The command:

```
# modprobe nf_log_ipv4
```

loads the ipv4 log module. If multiple log modules are loaded you can set the preferred/active logger with sysctl. For example:

```
# sudo sysctl net.netfilter.nf_log.2=nf_log_ipv4
```

tells the kernel to log ipv4 packet events to syslog/journald. This only affects log messages generated by conntrack debugging. Log messages generated by rules like “*ipables* *-j NFLOG*” or the *LOG* target do not change as the rule itself already specifies to log type to use (nfnetlink and syslog/journald respectively).

After this, debug messages will appear in ulogd (if configured via nfnetlink) or the system log (if nf_log_ipv4 is the log backend).

## Example debug output

The following examples occur with the settings created using *“sudo sysctl net.netfilter.nf_log.2=nf_log_ipv4”* and “*sudo sysctl net.netfilter.nf_conntrack_log_invalid=6*“.

```
nf_ct_proto_6: invalid packet ignored in state ESTABLISHED SRC=10.47.217.34 DST=192.168.0.17 LEN=60 DF SPT=443 DPT=47832 SEQ=389276 ACK=3130 WINDOW=65160 ACK SYN

nf_ct_proto_6: ACK is over the upper bound (ACKed data not seen yet) SRC=10.3.1.1 DST=192.168.0.1 LEN=60 DF SPT=443 DPT=49135 SEQ= ...
```

This dump contains the packet contents (allowing correlation with tcpdump packet capture of the flow, for example) plus a reason why the packet was tagged as INVALID.

## Dynamic Printk

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

Each line shows the location of a default-disabled debug *printk* statement. *printk* is a C function from the Linux kernel interface that prints messages to the kernel log. The name of the file in the linux kernel source code comes first, followed by the line number. The square brackets contain the name of the kernel module that this source file is part of. The combination of file name and line number allows enabling or disabling these *printk* statements. This command:

```
# sudo echo "file net/netfilter/nf_conntrack_proto_tcp.c line 1005 +p" &gt; /sys/kernel/debug/dynamic_debug/control
```

will enable the *printk* statement shown in line 1005 of [net/netfilter/nf_conntrack_proto_tcp.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/net/netfilter/nf_conntrack_proto_tcp.c?id=e2ef5203c817a60bfb591343ffd851b6537370ff#n1005). The same command, with “*+p*” replaced by “*-p*“, disables this log line again. This facility is not unique to connection tracking: many parts of the kernel provide such debug messages. This technique is useful when things go wrong and more information about the conntrack internal state is needed. A dedicated howto about the dynamic debug feature is available in the kernel documentation [here](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/dynamic-debug-howto.rst?id=e85d92b3bc3b7062f18b24092a65ec427afa8148).

## The unconfirmed and dying lists

A newly allocated conntrack entry is first added to the unconfirmed list. Once the packet is accepted by all iptables/nftables rules, the conntrack entry moves from the unconfirmed list to the main connection tracking table. The dying list is the inverse: when a entry is removed from the table, it is placed on the dying list. The entry is freed once all packets that reference the flow have been dropped. This means that a conntrack entry is always on a list: Either the unconfirmed list, the dying list, or the conntrack hash table list. Most entries will be in the hash table.

If removal from the table is due to a timeout, no further references exist and the entry is freed immediately. This is what will typically happen with UDP flows. For TCP, conntrack entries are normally removed due to a special TCP packets such as the last TCP acknowledgment or a TCP reset. This is because TCP, unlike UDP, signals state transitions, such as connection closure. The entry is moved from the table to the dying list. The conntrack entry is then released after the network stack has processed the “last packet” packet.

### Examining these lists

```
# sudo conntrack -L unconfirmed
# sudo conntrack -L dying
```

These two commands show the lists. A large discrepancy between the number of active connections (*sudo conntrack -C*) and the content of the connection tracking table (*sudo conntrack -L*) indicate a problem. Entries that remain on either one of these lists for long time periods indicate a kernel bug. Expected time ranges are in the order of a few microseconds.

## Summary

This article gave an introduction to several debugging aids that can be helpful to pinpoint problems with the connection tracking module.

