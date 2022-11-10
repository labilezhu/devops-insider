# Fun with network namespaces, part 1

> [https://www.gilesthomas.com/2021/03/fun-with-network-namespaces](https://www.gilesthomas.com/2021/03/fun-with-network-namespaces)



Linux has some amazing kernel features to enable containerization. Tools like [Docker](https://www.docker.com/) are built on top of them, and at [PythonAnywhere](https://www.pythonanywhere.com/) we have built our own virtualization system using them.

One part of these systems that I've not spent much time poking into is network namespaces. [Namespaces](https://en.wikipedia.org/wiki/Linux_namespaces) are a general abstraction that allows you to separate out system resources; for example, if a process is in a *mount namespace*, then it has its own set of mounted disks that is separate from those seen by the other processes on a machine -- or if it's in a [*process namespace*](https://www.gilesthomas.com/2016/04/pam-unshare-a-pam-module-that-switches-into-a-pid-namespace), then it has its own cordoned-off set of processes visible to it (so, say, `ps auxwf` will just show the ones in its namespace).

As you might expect from that, if you put a process into a network namespace, it will have its own restricted view of what the networking environment looks like -- it won't see the machine's main network interface,

This provides certain advantages when it comes to security, but one that I thought was interesting is that because two processes inside different namespaces would have different networking environments, they could both bind to the same port -- and then could be accessed from outside via port forwarding.

To put that in more concrete terms: my goal was to be able to start up two Flask servers on the same machine, both bound to port 8080 inside their own namespace. I wanted to be able to access one of them from outside by hitting port 6000 on the machine, and the other by hitting port 6001.

Here is a run through how I got that working; it's a lightly-edited set of my "lab notes".



## Creating a network namespace and looking inside

The first thing to try is just creating a network namespace, called `netns1`:

```
# ip netns add netns1
```

Now, you can "go into" the created namespace by using `ip netns exec` *namespace-name*, so we can run Bash there and then use `ip a` to see what network interfaces we have available:

```
# ip netns exec netns1 /bin/bash
# ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
# exit
```

(I'll put the `ip netns exec` command at the start of all code blocks below if the block in question needs to be run inside the namespace, even when it's not necessary, so that it's reasonably clear which commands are to be run inside and which are not.)

So, we have a new namespace, and when we're inside it, there's only one interface available, a basic loopback interface. We can compare that with what we see with the same command outside:

```
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: ens5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9001 qdisc mq state UP group default qlen 1000
    link/ether 0a:d6:01:7e:06:5b brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.173/24 brd 10.0.0.255 scope global dynamic ens5
       valid_lft 2802sec preferred_lft 2802sec
    inet6 fe80::8d6:1ff:fe7e:65b/64 scope link
       valid_lft forever preferred_lft forever
```

There we can see the actual network card attached to the machine, which has the name `ens5`.

## Getting the loopback interface working

You might have noticed that the details shown for the loopback interface inside the namespace were much shorter, too -- no IPv4 or IPv6 addresses, for example. That's because the interface is down by default. Let's see if we can fix that:

```
# ip netns exec netns1 /bin/bash
# ip a
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
# ping 127.0.0.1
ping: connect: Network is unreachable
# ip link set dev lo up
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
# ping 127.0.0.1
PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.019 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.027 ms
^C
--- 127.0.0.1 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1022ms
rtt min/avg/max/mdev = 0.019/0.023/0.027/0.004 ms
```

So, we could not ping the loopback when it was down (unsurprisingly) but once we used the `ip link set dev lo up` command, it showed up as configured and was pingable.

Now we have a working loopback interface, but the external network still is down:

```
# ip netns exec netns1 /bin/bash
# ping 8.8.8.8
ping: connect: Network is unreachable
```

Again, that makes sense. There's no non-loopback interface, so there's no way to send packets to anywhere but the loopback network.

## Virtual network interfaces: connecting the namespace

What we need is some kind of non-loopback network interface inside the namespace. However, we can't just put the external interface `ens5` inside there; an interface can only be in one namespace at a time, so if we put that one in there, the external machine would lose networking.

What we need to do is create a virtual network interface. These are created in pairs, and are essentially connected to each other. This command:

```
# ip link add veth0 type veth peer name veth1
```

Creates interfaces called `veth0` and `veth1`. Anything sent to `veth0` will appear on `veth1`, and vice versa. It's as if they were two separate ethernet cards, connected to the same hub (but not to anything else). Having run that command (outside the network namespace) we can list all of our available interfaces:

```
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: ens5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9001 qdisc mq state UP group default qlen 1000
    link/ether 0a:d6:01:7e:06:5b brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.173/24 brd 10.0.0.255 scope global dynamic ens5
       valid_lft 2375sec preferred_lft 2375sec
    inet6 fe80::8d6:1ff:fe7e:65b/64 scope link
       valid_lft forever preferred_lft forever
5: veth1@veth0: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether ce:d5:74:80:65:08 brd ff:ff:ff:ff:ff:ff
6: veth0@veth1: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 22:55:4e:34:ce:ba brd ff:ff:ff:ff:ff:ff
```

You can see that they're both there, and are currently down. I read the `veth1@veth0` notation as meaning "virtual interface `veth1`, which is connected to the virtual interface `veth0`".

We can now move one of them -- `veth1` -- into the network namespace `netns1`, which means that we have the interface outside connected to the one inside:

```
# ip link set veth1 netns netns1
```

Now, from outside, we see this:

```
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: ens5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9001 qdisc mq state UP group default qlen 1000
    link/ether 0a:d6:01:7e:06:5b brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.173/24 brd 10.0.0.255 scope global dynamic ens5
       valid_lft 2368sec preferred_lft 2368sec
    inet6 fe80::8d6:1ff:fe7e:65b/64 scope link
       valid_lft forever preferred_lft forever
6: veth0@if5: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 22:55:4e:34:ce:ba brd ff:ff:ff:ff:ff:ff link-netns netns1
```

`veth1` has disappeared (and `veth0` is now `@if5`, which is interesting -- not sure why, though it seems to make some kind of sense given that `veth1` is now inside another namespace). But anyway, inside, we can see our moved interface:

```
root@giles-devweb1:~# ip netns exec netns1 /bin/bash
root@giles-devweb1:~# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
5: veth1@if6: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether ce:d5:74:80:65:08 brd ff:ff:ff:ff:ff:ff link-netnsid 0
```

At this point we have a network interface outside the namespace, which is connected to an interface inside. However, in order to actually use them, we'll need to bring the interfaces up and set up routing. The first step is to bring the outside one up; we'll give it the IP address `192.168.0.1` on the `192.168.0.0/24` subnet (that is, the network covering all addresses from `192.168.0.0` to `192.168.0.255`)

```
# ip addr add 192.168.0.1/24 dev veth0
# ip link set dev veth0 up
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: ens5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 9001 qdisc mq state UP group default qlen 1000
    link/ether 0a:d6:01:7e:06:5b brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.173/24 brd 10.0.0.255 scope global dynamic ens5
       valid_lft 3567sec preferred_lft 3567sec
    inet6 fe80::8d6:1ff:fe7e:65b/64 scope link
       valid_lft forever preferred_lft forever
6: veth0@if5: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state LOWERLAYERDOWN group default qlen 1000
    link/ether 22:55:4e:34:ce:ba brd ff:ff:ff:ff:ff:ff link-netns netns1
    inet 192.168.0.1/24 scope global veth0
       valid_lft forever preferred_lft forever
```

So that's all looking good; it reports "no carrier" at the moment, of course, because there's nothing at the other end yet. Let's go into the namespace and sort that out by bringing it up on `192.168.0.2` on the same network:

```
# ip netns exec netns1 /bin/bash
# ip addr add 192.168.0.2/24 dev veth1
# ip link set dev veth1 up
# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
5: veth1@if6: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether ce:d5:74:80:65:08 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.0.2/24 scope global veth1
       valid_lft forever preferred_lft forever
    inet6 fe80::ccd5:74ff:fe80:6508/64 scope link tentative
       valid_lft forever preferred_lft forever
```

Now, let's try pinging from inside the namespace to the outside interface:

```
# ip netns exec netns1 /bin/bash
# ping 192.168.0.1
PING 192.168.0.1 (192.168.0.1) 56(84) bytes of data.
64 bytes from 192.168.0.1: icmp_seq=1 ttl=64 time=0.069 ms
64 bytes from 192.168.0.1: icmp_seq=2 ttl=64 time=0.042 ms
^C
--- 192.168.0.1 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1024ms
rtt min/avg/max/mdev = 0.042/0.055/0.069/0.013 ms
```

And from outside to the inside:

```
# ping 192.168.0.2
PING 192.168.0.2 (192.168.0.2) 56(84) bytes of data.
64 bytes from 192.168.0.2: icmp_seq=1 ttl=64 time=0.039 ms
64 bytes from 192.168.0.2: icmp_seq=2 ttl=64 time=0.043 ms
^C
--- 192.168.0.2 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1018ms
rtt min/avg/max/mdev = 0.039/0.041/0.043/0.002 ms
```

Great!

However, of course, it's still not routed -- from inside the interface, we still can't ping Google's DNS server:

```
# ip netns exec netns1 /bin/bash
# ping 8.8.8.8
ping: connect: Network is unreachable
```

## Connecting the namespace to the outside world with NAT

We need to somehow connect the network defined by our pair of virtual interfaces to the one that is accessed via our real hardware network interface, either by setting up bridging or NAT. I'm running this experiment on a machine on AWS, and I'm not sure how well that would work with bridging (my guess is, really badly), so let's go with NAT.

First we tell the network stack inside the namespace to route everything via the machine at the other end of the connection defined by its internal `veth1` IP address:

```
# ip netns exec netns1 /bin/bash
# ip route add default via 192.168.0.1
# ping 8.8.8.8
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
^C
--- 8.8.8.8 ping statistics ---
3 packets transmitted, 0 received, 100% packet loss, time 2028ms
```

Note that the address we specify in the `ip route add default` command is the one for the end of the virtual interface pair that is *outside* the process namespace, which makes sense -- we're saying "this other machine is our router". The first time I tried this I put the address of the interface inside the namespace there, which obviously didn't work, as it was trying to send packets to itself for routing.

So now the networking stack inside the namespace knows where to route stuff, which is why it no longer says "Network is unreachable", but of course there's nothing on the other side to send it onwards, so our ping packets are getting dropped on the floor. We need to use `iptables` to set up that side of things outside the namespace.

The first step is to tell the host that it can route stuff:

```
# cat /proc/sys/net/ipv4/ip_forward
0
# echo 1 > /proc/sys/net/ipv4/ip_forward
# cat /proc/sys/net/ipv4/ip_forward
1
```

Now that we're forwarding packets, we want to make sure that we're not just forwarding them willy-nilly around the network. If we check the current rules in the FORWARD chain (in the default "filter" table):

```
# iptables -L FORWARD
Chain FORWARD (policy ACCEPT)
target     prot opt source               destination
#
```

We see that the default is ACCEPT, so we'll change that to DROP:

```
# iptables -P FORWARD DROP
# iptables -L FORWARD
Chain FORWARD (policy DROP)
target     prot opt source               destination
#
```

OK, now we want to make some changes to the `nat` iptable so that we have routing. Let's see what we have first:

```
# iptables -t nat -L
Chain PREROUTING (policy ACCEPT)
target     prot opt source               destination
DOCKER     all  --  anywhere             anywhere             ADDRTYPE match dst-type LOCAL

Chain INPUT (policy ACCEPT)
target     prot opt source               destination

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination
DOCKER     all  --  anywhere            !localhost/8          ADDRTYPE match dst-type LOCAL

Chain POSTROUTING (policy ACCEPT)
target     prot opt source               destination
MASQUERADE  all  --  ip-172-17-0-0.ec2.internal/16  anywhere

Chain DOCKER (2 references)
target     prot opt source               destination
RETURN     all  --  anywhere             anywhere
#
```

I have Docker installed on the machine already, and it's got some of its own NAT-based routing configured there. I don't think there's any harm in leaving that there; it's on a different subnet to the one I chose for my own stuff.

So, firstly, we'll enable masquerading from the `192.168.0.*` network onto our main ethernet interface `ens5`:

```
# iptables -t nat -A POSTROUTING -s 192.168.0.0/255.255.255.0 -o ens5 -j MASQUERADE
```

Now we'll say that we'll forward stuff that comes in on `ens5` can be forwarded to our `veth0` interface, which you'll remember is the end of the virtual network pair that is outside the namespace:

```
# iptables -A FORWARD -i ens5 -o veth0 -j ACCEPT
```

...and then the routing in the other direction:

```
# iptables -A FORWARD -o ens5 -i veth0 -j ACCEPT
```

Now, let's see what happens if we try to ping from inside the namespace

```
# ip netns exec netns1 /bin/bash
# ping 8.8.8.8
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
64 bytes from 8.8.8.8: icmp_seq=1 ttl=112 time=0.604 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=112 time=0.609 ms
^C
--- 8.8.8.8 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1003ms
rtt min/avg/max/mdev = 0.604/0.606/0.609/0.002 ms
```

w00t!

## Running a server with port-forwarding

Right, now we have a network namespace where we can operate as a network client -- processes running inside it can access the external Internet.

However, we don't have things working the other way around; we cannot run a server inside the namespace and access it from outside. For that, we need to configure port-forwarding. I'm not perfectly clear in my own mind exactly how this all works; take my explanations below with a cellar of salt...

We use the "[Destination NAT](http://linux-ip.net/html/nat-dnat.html)" chain in iptables:

```
# iptables -t nat -A PREROUTING -p tcp -i ens5 --dport 6000 -j DNAT --to-destination 192.168.0.2:8080
```

Or, in other words, if something comes in for port `6000` then we should sent it on to port 8080 on the interface at `192.168.0.2` (which is the end of the virtual interface pair that is inside the namespace).

Next, we say that we're happy to forward stuff back and forth over new, established and related (not sure what that last one is) connections to the IP of our namespaced interface:

```
# iptables -A FORWARD -p tcp -d 192.168.0.2 --dport 8080 -m state --state NEW,ESTABLISHED,RELATED -j ACCEPT
```

So, with that set up, we should be able to run a server inside the namespace on port 8080. Using this Python code in the file `server.py`

```
from flask import Flask

app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello from Flask!\n'

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=8080)
```

...then we run it:

```
# ip netns exec netns1 /bin/bash
# python3.7 server.py
 * Serving Flask app "server" (lazy loading)
 * Environment: production
   WARNING: This is a development server. Do not use it in a production deployment.
   Use a production WSGI server instead.
 * Debug mode: off
 * Running on http://0.0.0.0:8080/ (Press CTRL+C to quit)
```

...and from a completely separate machine on the same network as the one where we're running the server, we `curl` it using the machine's external IP address, on port `6000`:

```
$ curl http://10.0.0.233:6000/
Hello from Flask!
$
```

Yay! We've successfully got a Flask server running inside a network namespace, with routing from the external network.

## Running a second server in a separate namespace

Now we can set up the second server in its own namespace. Leaving the existing Flask running in the session where we started it just now, we can run through all of the steps above at speed in another:

```
# ip netns add netns2
# ip link add veth2 type veth peer name veth3
# ip link set veth3 netns netns2
# ip addr add 192.168.1.1/24 dev veth2
# ip link set dev veth2 up
# iptables -t nat -A POSTROUTING -s 192.168.1.0/255.255.255.0 -o ens5 -j MASQUERADE
# iptables -A FORWARD -i ens5 -o veth2 -j ACCEPT
# iptables -A FORWARD -o ens5 -i veth2 -j ACCEPT
# iptables -t nat -A PREROUTING -p tcp -i ens5 --dport 6001 -j DNAT --to-destination 192.168.1.2:8080
# iptables -A FORWARD -p tcp -d 192.168.1.2 --dport 8080 -m state --state NEW,ESTABLISHED,RELATED -j ACCEPT

# ip netns exec netns2 /bin/bash
# ip link set dev lo up
# ip addr add 192.168.1.2/24 dev veth3
# ip link set dev veth3 up
# ip route add default via 192.168.1.1
# python3.7 server2.py
 * Serving Flask app "server2" (lazy loading)
 * Environment: production
   WARNING: This is a development server. Do not use it in a production deployment.
   Use a production WSGI server instead.
 * Debug mode: off
 * Running on http://0.0.0.0:8080/ (Press CTRL+C to quit)
```

...where `server2.py` is the same Python code, modified to return a slightly different string. Now, from our other server:

```
$ curl http://10.0.0.233:6000/
Hello from Flask!
$ curl http://10.0.0.233:6001/
Hello from the other Flask!
$
```

And we're done :-) We have two separate servers on the same machine, both of which are bound to port `8080` inside their own network namespace, with routing set up so they can connect out to the external network, and port-forwarding so that they can be accessed from outside.

## Summing up

This was easier than I thought it was going to be; the only part of the networking that is still fuzzy in my own mind is the port-forwarding, and I think I just need to read up a bit on exactly how that works.

It was made a lot easier by various other tutorials on network namespaces that I found around the Internet. Some shout-outs:

- I felt [this LWN post](https://lwn.net/Articles/580893/) summarised the basics really well.
- [Ivan Sim's article on itnext.io](https://itnext.io/create-your-own-network-namespace-90aaebc745d) had some good hints and tips.
- As did [Diego Pino García's at Unweaving the Web](https://blogs.igalia.com/dpino/2016/04/10/network-namespaces/).
- ...and [Scott Lowes's blog was also useful](https://blog.scottlowe.org/2013/09/04/introducing-linux-network-namespaces/).
- My port-forwarding setup was a good example of [full-stack-overflow programming](https://serverfault.com/questions/140622/how-can-i-port-forward-with-iptables).
- The [`iptables` man page](https://linux.die.net/man/8/iptables) clarified a bunch of stuff for me.

Many thanks to all of them!
