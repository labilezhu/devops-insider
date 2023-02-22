
### Could not resolve symbol: /proc/self/exe:BEGIN_trigger

```bash
labile@labile-T30 ➜ pub-diy $ sudo bpftrace /home/labile/pub-diy/low-tec/trace/trace-istio/sidecar-outbound-local-port-collisions/trace-poll-timeout.bt
Attaching 13 probes...
ERROR: Could not resolve symbol: /proc/self/exe:BEGIN_trigger
```

> https://github.com/iovisor/bpftrace/issues/2168
> https://wiki.ubuntu.com/Debug%20Symbol%20Packages
> https://github.com/iovisor/bpftrace/pull/2264

```bash
echo "deb http://ddebs.ubuntu.com $(lsb_release -cs) main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-updates main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-proposed main restricted universe multiverse" | \
sudo tee -a /etc/apt/sources.list.d/ddebs.list
sudo apt install ubuntu-dbgsym-keyring
sudo apt update
sudo apt install bpftrace-dbgsym
```


