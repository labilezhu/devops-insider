# MTU

```{toctree}
check-mtu.md
```

## Disable Path MTU discovery for specify ip/subnet by setting it manually

> [https://tldp.org/HOWTO/Adv-Routing-HOWTO/lartc.cookbook.mtu-discovery.html](https://tldp.org/HOWTO/Adv-Routing-HOWTO/lartc.cookbook.mtu-discovery.html)

```bash
ip route add default via 10.0.0.1 mtu 296
```

In general, it is possible to override PMTU Discovery by setting specific routes. For example, if only a certain subnet is giving problems, this should help:

```bash
ip route add 195.96.96.0/24 via 10.0.0.1 mtu 1000
```