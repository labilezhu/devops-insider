---
title: Conntrack tales - one thousand and one flows
date: 2021-07-20T23:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
---

# Conntrack tales - one thousand and one flows

> https://blog.cloudflare.com/conntrack-tales-one-thousand-and-one-flows/

At Cloudflare we develop new products at a great pace. Their needs often challenge the architectural assumptions we made in the past. For example, years ago we decided to avoid using Linux's "conntrack" - stateful firewall facility. This brought great benefits - it simplified our iptables firewall setup, sped up the system a bit and made the inbound packet path easier to understand.

But eventually our needs changed. One of our new products had a reasonable need for it. But we weren't confident - can we just enable conntrack and move on? How does it actually work? I volunteered to help the team understand the dark corners of the "conntrack" subsystem.

## What is conntrack?

"Conntrack" is a part of Linux network stack, specifically part of the firewall subsystem. To put that into perspective: early firewalls were entirely stateless. They could express only basic logic, like: allow SYN packets to port 80 and 443, and block everything else.

The stateless design gave some basic network security, but was quickly deemed insufficient. You see, there are certain things that can't be expressed in a stateless way. The canonical example is assessment of ACK packets - it's impossible to say if an ACK packet is legitimate or part of a port scanning attempt, without tracking the connection state.

To fill such gaps all the operating systems implemented connection tracking inside their firewalls. This tracking is usually implemented as a big table, with at least 6 columns: protocol (usually TCP or UDP), source IP, source port, destination IP, destination port and connection state. On Linux this subsystem is called "conntrack" and is often enabled by default. Here's how the table looks on my laptop inspected with "conntrack -L" command:

![img](conntrack-tales@cloudflare.assets/image5-2.png)

The obvious question is how large this state tracking table can be. This setting is under "/proc/sys/net/nf_conntrack_max":

```undefined
$ cat /proc/sys/net/nf_conntrack_max
262144
```

This is a global setting, but the limit is per container. On my system each container, or "network namespace", can have up to 256K conntrack entries.

What exactly happens when the number of concurrent connections exceeds the conntrack limit?

## Testing conntrack is hard

In past testing conntrack was hard - it required complex hardware or vm setup. Fortunately, these days we can use modern "user namespace" facilities which do permission magic, allowing an unprivileged user to feel like root. Using the tool "unshare" it's possible to create an isolated environment where we can precisely control the packets going through and experiment with iptables and conntrack without threatening the health of our host system. With appropriate parameters it's possible to create and manage a networking namespace, including access to namespaced iptables and conntrack, from an unprivileged user.

This script is the heart of our test:

```bash
# Enable tun interface
ip tuntap add name tun0 mode tun
ip link set tun0 up
ip addr add 192.0.2.1 peer 192.0.2.2 dev tun0
ip route add 0.0.0.0/0 via 192.0.2.2 dev tun0

# Refer to conntrack at least once to ensure it's enabled
iptables -t raw -A PREROUTING -j CT
# Create a counter in mangle table
iptables -t mangle -A PREROUTING
# Make sure reverse traffic doesn't affect conntrack state
iptables -t raw -A OUTPUT -p tcp --sport 80 -j DROP

tcpdump -ni any -B 16384 -ttt &
...
./venv/bin/python3 send_syn.py

conntrack -L
# Show iptables counters
iptables -nvx -t raw -L PREROUTING
iptables -nvx -t mangle -L PREROUTING
```

This bash script is shortened for readability. See the [full version here](https://github.com/cloudflare/cloudflare-blog/blob/master/2020-04-conntrack-syn/test-1.bash). The accompanying "send_syn.py" is just sending 10 SYN packets over "tun0" interface. [Here is the source](https://github.com/cloudflare/cloudflare-blog/blob/master/2020-04-conntrack-syn/send_syn.py) but allow me to paste it here - showing off "scapy" is always fun:

```undefined
tun = TunTapInterface("tun0", mode_tun=True)
tun.open()

for i in range(10000,10000+10):
    ip=IP(src="198.18.0.2", dst="192.0.2.1")
    tcp=TCP(sport=i, dport=80, flags="S")
    send(ip/tcp, verbose=False, inter=0.01, socket=tun)
```

The bash script above contains a couple of gems. Let's walk through them.

First, please note that we can't just inject packets into the loopback interface using [SOCK_RAW sockets](http://man7.org/linux/man-pages/man7/raw.7.html). The Linux networking stack is a complex beast. The semantics of sending packets over a SOCK_RAW are different then delivering a packet over a real interface. We'll discuss this later, but for now, to avoid triggering unexpected behaviour, we will deliver packets over a tun/tap device which better emulates a real interface.

Then we need to make sure the conntrack is active in the network namespace we wish to use for testing. Traditionally, just loading the kernel module would have done that, but in the brave new world of containers and network namespaces, a method had to be found to allow conntrack to be active in some and inactive in other containers. Hence this is tied to usage - rules referencing conntrack must exist in the namespace's iptables for conntrack to be active inside the container.

As a side note, [containers triggering host to load kernel modules](https://lwn.net/Articles/740455/) is an [interesting subject](https://github.com/weaveworks/go-odp/blob/6b0aa22550d9325eb8f43418185859e13dc0de1d/odp/dpif.go#L67-L90).

After the "-t raw -A PREROUTING" rule, which we added "-t mangle -A PREROUTING" rule, but notice - it doesn't have any action! This syntax is allowed by iptables and it is pretty useful to get iptables to report rule counters. We'll need these counters soon. A careful reader might suggest looking at "policy" counters in iptables to achieve our goal. Sadly, "policy" counters (increased for each packet entering a chain), work only if there is at least one rule inside it.

The rest of the steps are self-explanatory. We set up "tcpdump" in the background, send 10 SYN packets to 127.0.0.1:80 using the "scapy" Python library. Then we print the conntrack table and iptables counters.

Let's run this script in action. Remember to run it under networking namespace as fake root with "unshare -Ur -n":

![img](conntrack-tales@cloudflare.assets/image6.png)

This is all nice. First we see a "tcpdump" listing showing 10 SYN packets. Then we see the conntrack table state, showing 10 created flows. Finally, we see iptables counters in two rules we created, each showing 10 packets processed.

## Can conntrack table fill up?

Given that the conntrack table is size constrained, what exactly happens when it fills up? Let's check it out. First, we need to drop the conntrack size. As mentioned it's controlled by a global toggle - it's necessary to tune it on the host side. Let's reduce the table size to 7 entries, and repeat our test:

![img](conntrack-tales@cloudflare.assets/image4-3.png)

This is getting interesting. We still see the 10 inbound SYN packets. We still see that the "-t raw PREROUTING" table received 10 packets, but this is where similarities end. The "-t mangle PREROUTING" table saw only 7 packets. Where did the three missing SYN packets go?

It turns out they went where all the dead packets go. They were hard dropped. Conntrack on overfill does exactly that. It even complains in the "dmesg":

![image1-1](conntrack-tales@cloudflare.assets/image1-1.png)

This is confirmed by our iptables counters. Let's review the [famous iptables](https://upload.wikimedia.org/wikipedia/commons/3/37/Netfilter-packet-flow.svg) diagram:

![img](conntrack-tales@cloudflare.assets/image7.png)[image](https://commons.wikimedia.org/wiki/File:Netfilter-packet-flow.svg) by [Jan Engelhardt](https://commons.wikimedia.org/wiki/User_talk:Jengelh) CC BY-SA 3.0

As we can see, the "-t raw PREROUTING" happens before conntrack, while "-t mangle PREROUTING" is just after it. This is why we see 10 and 7 packets reported by our iptables counters.

Let me emphasize the gravity of our discovery. We showed three completely valid SYN packets being implicitly dropped by "conntrack". There is no explicit "-j DROP" iptables rule. There is no configuration to be toggled. Just the fact of using "conntrack" means that, when it's full, packets creating new flows will be dropped. No questions asked.

This is the dark side of using conntrack. If you use it, you absolutely must make sure it doesn't get filled.

We could end our investigation here, but there are a couple of interesting caveats.

## Strict vs loose

Conntrack supports a "strict" and "loose" mode, as configured by "nf_conntrack_tcp_loose" toggle.

```
$ cat /proc/sys/net/netfilter/nf_conntrack_tcp_loose
1
```

By default, it's set to "loose" which means that stray(流浪) ACK packets for unseen TCP flows will create new flow entries in the table. We can generalize(概括): "conntrack" will implicitly drop all the packets that create new flow, whether that's SYN or just stray ACK.

What happens when we clear the "nf_conntrack_tcp_loose=0" setting? This is a subject for another blog post, but suffice to say(但我只想说) - it's a mess(混乱). First, this setting is not settable in the network namespace scope - although it should be. To test it you need to be in the root network namespace. Then, due to twisted logic the ACK will be dropped on a full conntrack table, even though in this case it doesn't create a flow. If the table is not full, the ACK packet will pass through it, having "-ctstate INVALID" from "mangle" table forward.

## When doesn't a conntrack entry get created?

There are important situations when conntrack entry is not created. For example, we could replace these line in our script:

```
# Make sure reverse traffic doesn't affect conntrack state
iptables -t raw -A OUTPUT -p tcp --sport 80 -j DROP
```

With those:

```
# Make sure inbound SYN packets don't go to networking stack
iptables -A INPUT -j DROP
```

Naively we could think dropping SYN packets past the conntrack layer would not interfere with the created flows. This is not correct. In spite of these SYN packets having been seen by conntrack, no flow state is created for them. Packets hitting "-j DROP" will not create new conntrack flows. Pretty magical, isn't it?

## Full Conntrack causes with EPERM

Recently we hit a case when a "sendto()" syscall on UDP socket from one of our applications was erroring with EPERM. This is pretty weird, and not documented in the man page. My colleague had no doubts:

![image9-1](conntrack-tales@cloudflare.assets/image9-1.png)

I'll save you the gruesome details, but indeed, the full conntrack table will do that to your new UDP flows - you will get EPERM. Beware. Funnily enough, it's possible to get EPERM if an outbound packet is dropped on OUTPUT firewall in other ways. For example:

```
marek:~$ sudo iptables -I OUTPUT -p udp --dport 53 --dst 192.0.2.8 -j DROP
marek:~$ strace -e trace=write nc -vu 192.0.2.8 53
write(3, "X", 1)                        = -1 EPERM (Operation not permitted)
+++ exited with 1 +++
```

If you ever receive EPERM from "sendto()", you might want to treat it as a transient error, if you suspect a filled conntrack problem, or permanent error if you blame iptables configuration.

This is also why we can't send our SYN packets directly using SOCK_RAW sockets in our test. Let's see what happens on conntrack overfill with standard "hping3" tool:

```
$ hping3 -S -i u10000 -c 10 --spoof 192.18.0.2 192.0.2.1 -p 80 -I lo
HPING 192.0.2.1 (lo 192.0.2.1): S set, 40 headers + 0 data bytes
[send_ip] sendto: Operation not permitted
```

"send()" even on a SOCK_RAW socket fails with EPERM when conntrack table is full.

## Full conntrack can happen on a SYN flood

There is one more caveat. During a SYN flood, the conntrack entries will totally be created for the spoofed flows. Take a look at second test case we prepared, this time correctly listening on port 80, and sending SYN+ACK:

![img](conntrack-tales@cloudflare.assets/image8.png)

We can see 7 SYN+ACK's flying out of the port 80 listening socket. The final three SYN's go nowhere as they are dropped by conntrack.

This has important implications. If you use conntrack on publicly accessible ports, during SYN flood [mitigation technologies like SYN Cookies](https://blog.cloudflare.com/syn-packet-handling-in-the-wild/) won't help. You are still at risk of running out of conntrack space and therefore affecting legitimate connections.

For this reason, as a general rule consider avoiding conntrack on inbound connections (-j NOTRACK). Alternatively having some reasonable rate limits on iptables layer, doing "-j DROP". This will work well and won't create new flows, as we discussed above. The best method though, would be to trigger SYN Cookies from a layer before conntrack, like XDP. But this is a subject for another time.

## Summary

Over the years Linux conntrack has gone through many changes and has improved a lot. While performance used to be a major concern, these days it's considered to be very fast. Dark corners remain. Correctly applying conntrack is tricky.

In this blog post we showed how it's possible to test parts of conntrack with "unshare" and a series of scripts. We showed the behaviour when the conntrack table gets filled - packets might implicitly be dropped. Finally, we mentioned the curious case of SYN floods where incorrectly applied conntrack may cause harm.

Stay tuned for more horror stories as we dig deeper and deeper into the Linux networking stack guts.
