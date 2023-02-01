# Netfilter

```{toctree}
ref/nftables_packet_flow_netfilter_hooks_detail.md
```

## netfilter.org

### What is the netfilter.org project?

The netfilter project is a community-driven collaborative [FOSS](https://en.wikipedia.org/wiki/Free_and_open-source_software) project that provides packet filtering software for the [Linux](http://www.kernel.org/) 2.4.x and later kernel series. The netfilter project is commonly associated with [iptables](https://www.netfilter.org/projects/iptables/index.html) and its successor [nftables](https://www.netfilter.org/projects/nftables/index.html).

The netfilter project enables packet filtering, network address [and port] translation (NA[P]T), packet logging, userspace packet queueing and other packet mangling.

The netfilter hooks are a framework inside the Linux kernel that allows kernel modules to register callback functions at different locations of the Linux network stack. The registered callback function is then called back for every packet that traverses the respective hook within the Linux network stack.

[iptables](https://www.netfilter.org/projects/iptables/index.html) is a generic firewalling software that allows you to define rulesets. Each rule within an IP table consists of a number of classifiers (iptables matches) and one connected action (iptables target).

[nftables](https://www.netfilter.org/projects/nftables/index.html) is the successor of [iptables](https://www.netfilter.org/projects/iptables/index.html), it allows for much more flexible, scalable and performance packet classification. This is where all the fancy new features are developed.

### What is iptables?

iptables is the userspace command line program used to configure the Linux 2.4.x and later packet filtering ruleset. It is targeted towards system administrators.

Since Network Address Translation is also configured from the packet filter ruleset, iptables is used for this, too.

The iptables package also includes ip6tables. ip6tables is used for configuring the IPv6 packet filter.

### What is nftables?

nftables replaces the popular {ip,ip6,arp,eb}tables. This software provides a new in-kernel packet classification framework that is based on a network-specific Virtual Machine (VM) and a new nft userspace command line tool. nftables reuses the existing Netfilter subsystems such as the existing hook infrastructure, the connection tracking system, NAT, userspace queueing and logging subsystem.

This software also provides libnftables, the high-level userspace library that includes support for JSON, see man (3)libnftables for more information.

#### Running nftables

You require the following software in order to run the nft command line tool:

- Linux kernel since 3.13, although newer kernel versions are recommended.
- libmnl: the minimalistic Netlink library
- libnftnl: low level netlink userspace library
- nft: command line tool

nft syntax differs from {ip,ip6,eb,arp}tables. Moreover, there is a backward compatibility layer that allows you run iptables/ip6tables, using the same syntax, over the nftables infrastructure.

#### Main Features

- Network-specific VM: the **nft** command line tool compiles the ruleset into the VM bytecode in netlink format, then it pushes this into the kernel via the nftables Netlink API. When retrieving the ruleset, the VM bytecode in netlink format is decompiled back to its original ruleset representation. So **nft** behaves both as compiler and decompiler.
- High performance through maps and concatenations: Linear ruleset inspection doesn't scale up. Using maps and concatenations, you can structure your ruleset to reduce the number of rule inspections to find the final action on the packet to the bare minimum.
- Smaller kernel codebase. The intelligence is placed in userspace **nft** command line tool, which is considerably more complex than iptables in terms of codebase, however, in the midrun, this will potentially allow us to deliver new features by upgrading the userspace command line tool, with no need of kernel upgrades.
- Unified and consistent syntax for every support protocol family, contrary to xtables utilities, that are well-known to be full of inconsistencies.

## iptbales Concept
![](./netfilter.drawio.svg)



- [Iptables Tutorial 1.2.2](https://www.frozentux.net/iptables-tutorial/iptables-tutorial.html)



## Ref.

https://www.digitalocean.com/community/tutorials/a-deep-dive-into-iptables-and-netfilter-architecture 