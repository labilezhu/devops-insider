# tcpdump

## packet size


> [https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html](https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html)
> 
Older versions of tcpdump truncate packets to 68 or 96 bytes. If this is the case, use -s to capture full-sized packets:

```bash
$ tcpdump -i <interface> -s 65535 -w <file>
```https://www.wireshark.org/docs/wsug_html_chunked/AppToolstcpdump.html