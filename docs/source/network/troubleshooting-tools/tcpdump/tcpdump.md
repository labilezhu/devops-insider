# tcpdump

## packet size


> [https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html](https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html)
> 
Older versions of tcpdump truncate packets to 68 or 96 bytes. If this is the case, use -s to capture full-sized packets:

```bash
$ tcpdump -i <interface> -s 65535 -w <file>
```

https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html

## 根据 RST FIN SYN 过滤

```bash
Show all RST packets:
# tcpdump 'tcp[13] & 4 != 0'

Show all FIN packets:
# tcpdump 'tcp[13] & 1 != 0'

Show all SYN packets:
# tcpdump 'tcp[13] & 2 != 0'

tcpdump -i any -c 100 -vv 'tcp[13] & 1 != 0 or tcp[13] & 4 != 0' 
```


```bash
export MY_IP=119.142.139.39 #update it

$ip route get $MY_IP
#119.142.139.39 via 119.63.71.129 dev eth3 src 119.63.71.132 uid 1001
#    cache 
export INTERFACE=eth3 #update it by above output

sudo tcpdump -i $INTERFACE -c 9999 -w /tmp/tcpdump.pcap  "host $MY_IP and (tcp[13] & 1 != 0 or tcp[13] & 4 != 0 or tcp[13] & 2 != 0) " #only capture RST or FIN or SYN

```