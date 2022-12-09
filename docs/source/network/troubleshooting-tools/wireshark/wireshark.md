# Wireshark

## Wireshark User’s Guide
> [https://www.wireshark.org/docs/wsug_html_chunked/index.html](https://www.wireshark.org/docs/wsug_html_chunked/index.html)


### DisplayFilters
> [https://wiki.wireshark.org/DisplayFilters](https://wiki.wireshark.org/DisplayFilters)

### Building Display Filter Expressions
> [https://www.wireshark.org/docs/wsug_html_chunked/ChWorkBuildDisplayFilterSection.html](https://www.wireshark.org/docs/wsug_html_chunked/ChWorkBuildDisplayFilterSection.html)




## Protocol field name
### tcp field name

> [https://www.wireshark.org/docs/dfref/t/tcp.html](https://www.wireshark.org/docs/dfref/t/tcp.html)


## Common script

```java
// FIN / RST / SYN
tcp.flags.fin == 1 || tcp.flags.reset == 1 || tcp.flags.syn == 1

ip.addr==10.14.12.13 && tcp.flags.reset == 1 && tcp.port==8080

ip.addr==10.14.12.13 && tcp.port==8080

ip.addr==10.14.12.13 && tcp.port==8080 && tcp.flags.fin == 1

// TCP Keepalive
ip.addr==10.14.12.13 && tcp.port==8080 && tcp.analysis.keep_alive

// TCP Port number reused
ip.addr==10.14.12.13 && tcp.port==8080 && tcp.analysis.reused_ports

tcp.connection.rst
tcp.connection.fin_active	
tcp.connection.fin_passive	

// 主动 FIN
tcp.connection.fin_active || tcp.flags.reset == 1

// by packet index 
tcp.stream eq 1006
```

```{toctree}
tcp-segmentationoffload(TSO).md
```