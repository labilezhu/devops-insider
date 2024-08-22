# Creating WireGuard jails with Linux network namespaces

> Source: https://www.ismailzai.com/blog/creating-wireguard-jails-with-linux-network-namespaces



The [network namespace](https://man7.org/linux/man-pages/man8/ip-netns.8.html) is a Linux kernel feature that allows for the creation of isolated network environments within the same logical host. Network namespaces isolate common networking resources, including:

- network interfaces
- routing tables
- firewall rules
- IP protocol stacks
- DNS resolution

A process that is launched in a network namespace is oblivious to the network resources in other namespaces, which means we can use this setup to jail processes that should only communicate through a specific interface, say a VPN tunnel. If the tunnel is the only interface in that namespace, when the tunnel goes down, it effectively kills networking for all processes within the namespace.

Setup for the scenario above is pretty straightforward:

1. create a network namespace
2. create a VPN tunnel in the network namespace
3. run applications in the network namespace

You'll need to be running a Linux kernel that has network namespaces compiled in and you'll need the [iproute2](https://wiki.linuxfoundation.org/networking/iproute2) packages for your distribution. You can checkout my [wgjailr repository](https://github.com/moismailzai/wgjailr/) for complete code samples if you want to skip ahead.

Setting up the namespace is simple:

```bash
ip netns add vpn
```



This creates a new network namespace called `vpn`. We can validate that the command worked by listing all network namespaces on our host:

```bash
ip netns list
```



The ip utility's `-n` flag allows us to target a specific network namespace. For instance, to see the devices in the `vpn` namespace:

```bash
ip -n vpn add
```



For now, there will only be a loopback interface, which we can bring up:

```bash
ip -n vpn link set lo up
```



Next, we will create the vpn tunnel interface and move it to the new network namespace:

```bash
ip link add tun0 type wireguard
ip link set tun0 netns vpn
```

Copy

With the tunnel interface in the correct namespace, we can now configure it. We'll use the `ip netns exec` command to run arbitrary commands in the context of the `vpn` network namespace. The command we want to run is `wg`, which we'll use to configure the tunnel interface we created earlier. Because the `wg` command expects configuration in a format that's different than the configuration file VPN providers normally distribute, we'll use `wg-quick` to strip out anything `wg` won't understand:

```bash
ip netns exec vpn wg setconf tun0 <(wg-quick strip /path/to/your/vpn.conf)
```



Next, we'll set a local IP address for our VPN interface:

```bash
ip -n vpn a add 10.2.0.2/32 dev tun0
```



And finally, we'll bring the interface up:

```bash
ip -n vpn link set tun0 up
```



At this point, the VPN tunnel is active in the network namespace we created and all that's left to do is to update the routing tables and the DNS nameserver in the network namespace. Updating the routes is straightforward:

```bash
ip -n vpn route add default dev tun0
```



Updating the DNS server is a little trickier: we'll need to override the global `/etc/resolv.conf` by creating one specific to our network namespace. These are read from `/etc/netns/name_of_your_namespace`:

```bash
mkdir -p /etc/netns/vpn/ && echo "nameserver 10.2.0.1" > /etc/netns/vpn/resolv.conf
```



And that's all there is to it, we now have two completely walled-off networking stacks on the same host. Applications we run in the `vpn` network namespace will not be aware of the interfaces, routes, and other configuration in the root namespace.

This is generally what we want but what if we need to communicate between the two namespaces. For instance, suppose we're running an application with a web interface within the the network namespace and we want to access that interface from elsewhere on our network. Because the root network namespace and the new one are oblivious to each other, this requires some extra work.

A quick and dirty solution is to relay between the two namespaces using `socat`:

```bash
/usr/bin/socat tcp-listen:8080,fork,reuseaddr exec:'ip netns exec vpn socat STDIO "tcp-connect:127.0.0.1:8080"',nofork
```



But why is this possible, aren't these network namespaces meant to be oblivious to each other?

One of the great things about network namespaces is that we can execute processes within them. Here, `socat` listens for incoming TCP connections on port `8080` of the root namespace and forwards the incoming data to a process. The process receiving the data is created by the command `ip netns exec vpn socat STDIO "tcp-connect:127.0.0.1:8080"`, which runs another instance of `socat` within the `vpn` network namespace and connects it to the local loobpack interface on port `8080`. The communication between these two instances of `socat`, *processes,* effectively becomes communication between network namespaces.

We could achieve something similar by using a [veth](https://man7.org/linux/man-pages/man4/veth.4.html) pair, which are specifically designed to communicate between network namespaces. This would be much more robust but also require more setup.

On a modern Linux distribution, all of this is simple to automate with scripts and Systemd unit files, which you can see in action over in my [GitHub repository](https://github.com/moismailzai/wgjailr/).
