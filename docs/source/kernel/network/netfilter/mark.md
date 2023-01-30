# Netfilter Mark

>The mark is a 32 bits integer value attached to a network packet. Some network parts interacting with it (see below) can do bitwise operations on this value, it can then be interpreted between one single 32 bits value up to a collection of 32 flags, or a mix of flags and smaller values, depending on how one chooses to organise its use (`tc` can't do this). Of course this mark *exists only as long as it's handled by the Linux kernel*. It's only purely virtual and internal, as it can have no existence on the wire. Depending on where's it's used, it may be called firewall mark, fwmark or simply mark.
>
>Each network packet processed by the kernel, is handled by a structure called [`sk_buff`](https://wiki.linuxfoundation.org/networking/sk_buff), defined in [`linux/include/linux/skbuff.h`](http://elixir.free-electrons.com/linux/v4.14/source/include/linux/skbuff.h). This structure includes various meta-data related to the packet when applicable, like IPsec information if any, related conntrack entry once looked up, ... and also its [mark](https://elixir.bootlin.com/linux/v4.14/source/include/linux/skbuff.h#L809).
>
>Various parts of the network stack can read this mark, change behaviour based on its value or (re)write it, eg:
>
>- [`tc`](https://manpages.debian.org/stretch/iproute2/tc-fw.8.en.html),
>- the routing stack can have special rules set with [`ip rule`](https://manpages.debian.org/stretch/iproute2/ip-rule.8.en.html) (eg `ip rule add fwmark 1 lookup 42`), to alter its routing decisions with this fwmark (eg to use a routing table sending those packets to an other interface than default),
>- of course [`iptables`](https://manpages.debian.org/stretch/iptables/iptables-extensions.8.en.html#MARK),
>- its candidate successor [nftables](https://manpages.debian.org/stretch/nftables/nft.8.en.html#META_STATEMENT),
>
>and a few other places...
>
>The main goal of this mark is to have all these network parts interact with each other by using it as a kind of message. The [Packet flow in Netfilter and General Networking](https://en.wikipedia.org/wiki/Netfilter#/media/File:Netfilter-packet-flow.svg) can help see in what order those elements will receive handling of the packet and thus its mark.
>
>There are other related marks beside fwmark:
>
>- [`connmark`](https://elixir.bootlin.com/linux/v4.14/source/include/net/netfilter/nf_conntrack.h#L89), which isn't stored with a packet's sk_buff, but in a conntrack entry tracking packet *flows*. Its connmark can of course be used by iptables with its [`connmark`](https://manpages.debian.org/stretch/iptables/iptables-extensions.8.en.html#connmark) match and [`CONNMARK`](https://manpages.debian.org/stretch/iptables/iptables-extensions.8.en.html#CONNMARK) target, with an usage example there: [Netfilter Connmark To Linux and beyond !](https://home.regit.org/netfilter-en/netfilter-connmark/). It allows the decision made based on one single packet to be memorized and then applied to all the packets of the same connection.
>- [`secmark`](https://elixir.bootlin.com/linux/v4.14/source/include/linux/skbuff.h#L805) and likewise its associated [`connsecmark`](https://elixir.bootlin.com/linux/v4.14/source/include/net/netfilter/nf_conntrack.h#L92) which are intended to interact with Linux Security Modules such as [SELinux](https://lwn.net/Articles/184786/).

Ref. https://unix.stackexchange.com/questions/467076/how-set-mark-option-works-on-netfilter-iptables



## Set Mark

>The **MARK** target is used to set **Netfilter** mark values that are associated with specific packets. This target is only valid in the mangle table, and will not work outside there. The **MARK** values may be used in conjunction with the advanced routing capabilities in Linux to send different packets through different routes and to tell them to use different queue disciplines (qdisc), etc. For more information on advanced routing, check out the [*Linux Advanced Routing and Traffic Control HOW-TO*](https://www.linuxtopia.org/Linux_Firewall_iptables/a6831.html#LARTC). Note that the mark value is not set within the actual package, but is a value that is associated within the kernel with the packet. In other words, you can not set a **MARK** for a packet and then expect the **MARK** still to be there on another host. If this is what you want, you will be better off with the **TOS** target which will mangle the TOS value in the IP header.
>
>
>
>**Table 11-6. MARK target options**
>
>| Option      | **--set-mark**                                               |
>| ----------- | ------------------------------------------------------------ |
>| Example     | **iptables -t mangle -A PREROUTING -p tcp --dport 22 -j MARK --set-mark 2** |
>| Explanation | The **--set-mark** option is required to set a mark. The **--set-mark** match takes an integer value. For example, we may set mark 2 on a specific stream of packets, or on all packets from a specific host and then do advanced routing on that host, to decrease or increase the network bandwidth, etc. |

Ref. https://www.linuxtopia.org/Linux_Firewall_iptables/x4368.html



## Match condition

>```
>[!] --mark value[/mask]
>          Matches packets with the given unsigned mark value (if a mask is specified, this is logically ANDed with the mask before the comparison).
>```

Ref. iptables man page



