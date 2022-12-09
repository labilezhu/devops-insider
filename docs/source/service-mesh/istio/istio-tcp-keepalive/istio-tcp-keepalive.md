# Istio Gateway TCP Keepalive



:::{figure-md} 图：Istio Gateway TCP Keepalive 架构图
:class: full-width
<img src="/service-mesh/istio/istio-tcp-keepalive/istio-gw-external-load-balancer-topology.drawio.svg" alt="图：内核调度点与协作">

*图：Istio TCP Keepalive 架构图*
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Fistio-gw-external-load-balancer-topology.drawio.svg)*


增加 `EnvoyFilter`。 需要注意的是，增加以及同步到 envoy 后，确保 Envoy 有重启过才能生效！

```yaml
kubectl apply -f - <<"EOF"

apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter
metadata:
  name: mark-ingress-gateway-socket-options
spec:
  configPatches:
  - applyTo: LISTENER
    match:
      context: GATEWAY
      listener:
        name: 0.0.0.0_8080
        portNumber: 8080
    patch:
      operation: MERGE
      value:
        socket_options:
        - int_value: 1
          level: 1 #socket level
          name: 9 #SO_KEEPALIVE: enable keep-alive
          state: STATE_PREBIND
        - int_value: 300
          level: 6 #TCP
          name: 4 # TCP_KEEPIDLE
          state: STATE_PREBIND
        - int_value: 20
          level: 6 #TCP
          name: 5 # TCP_KEEPINTVL
          state: STATE_PREBIND
        - int_value: 4
          level: 6 #TCP
          name: 6 #TCP_KEEPCNT
          state: STATE_PREBIND

EOF
```




应用后检查:

```json 
kubectl exec  istio-ingressgateway-2nd49 -- curl localhost:15000/config_dump | less -N

   "dynamic_listeners": [
    {
     "name": "0.0.0.0_8080",
     "active_state": {
      "version_info": "2022-10-06T05:47:44Z/432",
      "listener": {
       "@type": "type.googleapis.com/envoy.config.listener.v3.Listener",
       "name": "0.0.0.0_8080",
       "address": {
        "socket_address": {
         "address": "0.0.0.0",
         "port_value": 8080
        }
       },
       "socket_options": [
        {
         "level": "1",
         "name": "9",
         "int_value": "1"
        },
        {
         "level": "6",
         "name": "4",
         "int_value": "300"
        },
        {
         "level": "6",
         "name": "5",
         "int_value": "20"
        },
        {
         "level": "6",
         "name": "6",
         "int_value": "4"
        }
       ],
      },
      ...
    }
```

检查实际连接：

```bash
$ ss -nte sport 8080 or dport 8080 

State    Recv-Q    Send-Q       Local Address:Port         Peer Address:Port
ESTAB    0         0            129.168.96.24:8080       129.168.164.66:41218    timer:(keepalive,4min48sec,0) uid:202507 ino:887707242 sk:aea1 <->
ESTAB    0         0            129.168.96.24:8080       129.168.83.247:41354    timer:(keepalive,4min35sec,0) uid:202507 ino:887711869 sk:aea2 <->
ESTAB    0         0            129.168.96.24:8080       129.168.164.66:41220    timer:(keepalive,4min53sec,0) uid:202507 ino:887706404 sk:aea3 <->

```

## socket opts

> [https://man7.org/linux/man-pages/man2/setsockopt.2.html](https://man7.org/linux/man-pages/man2/setsockopt.2.html)


```c
       int setsockopt(int sockfd, int level, int optname,
                      const void *optval, socklen_t optlen);
```

To manipulate options at the sockets API level, _level_ is specified as **SOL\_SOCKET**. To manipulate options at any other level the protocol number of the appropriate protocol controlling the option is supplied. For example, to indicate that an option is to be interpreted by the **TCP** protocol, _level_ should be set to the protocol number of **TCP**; see [getprotoent(3)](https://man7.org/linux/man-pages/man3/getprotoent.3.html).

即 `level` 是 `协议id` ，但 `socket级别(SOL_SOCKET：1) ` 除外。

### socket 级别的 opts 列表：

- [https://man7.org/linux/man-pages/man7/socket.7.html#:~:text=IPv6%20socket%20addresses).-,Socket%20options,-The%20socket%20options](https://man7.org/linux/man-pages/man7/socket.7.html#:~:text=IPv6%20socket%20addresses).-,Socket%20options,-The%20socket%20options)

其中：

```
       SO_KEEPALIVE
              Enable sending of keep-alive messages on connection-
              oriented sockets.  Expects an integer boolean flag.
```

编号：

[https://elixir.bootlin.com/linux/v5.4/source/include/uapi/asm-generic/socket.h#L9](https://elixir.bootlin.com/linux/v5.4/source/include/uapi/asm-generic/socket.h#L9)

```c
/* For setsockopt(2) */
#define SOL_SOCKET	1 //level

//opts
#define SO_DEBUG	1 
#define SO_REUSEADDR	2
#define SO_TYPE		3
#define SO_ERROR	4
#define SO_DONTROUTE	5
#define SO_BROADCAST	6
#define SO_SNDBUF	7
#define SO_RCVBUF	8
#define SO_SNDBUFFORCE	32
#define SO_RCVBUFFORCE	33
#define SO_KEEPALIVE	9 //开启 keepalive
#define SO_OOBINLINE	10
#define SO_NO_CHECK	11
#define SO_PRIORITY	12
#define SO_LINGER	13
#define SO_BSDCOMPAT	14
#define SO_REUSEPORT	15
#ifndef SO_PASSCRED /* powerpc only differs in these */
#define SO_PASSCRED	16
#define SO_PEERCRED	17
#define SO_RCVLOWAT	18
#define SO_SNDLOWAT	19
#define SO_RCVTIMEO_OLD	20
#define SO_SNDTIMEO_OLD	21
#endif
```



### TCP 级别的 opts 列表

- [https://man7.org/linux/man-pages/man7/tcp.7.html#:~:text=option%20from%20it.-,Socket%20options,-To%20set%20or](https://man7.org/linux/man-pages/man7/tcp.7.html#:~:text=option%20from%20it.-,Socket%20options,-To%20set%20or)

opts 编号：

[https://elixir.bootlin.com/linux/v5.4/source/include/uapi/linux/tcp.h#L97](https://elixir.bootlin.com/linux/v5.4/source/include/uapi/linux/tcp.h#L97)

```c
/* TCP socket options */
#define TCP_NODELAY		1	/* Turn off Nagle's algorithm. */
#define TCP_MAXSEG		2	/* Limit MSS */
#define TCP_CORK		3	/* Never send partially complete segments */
#define TCP_KEEPIDLE		4	/* Start keeplives after this period */
#define TCP_KEEPINTVL		5	/* Interval between keepalives */
#define TCP_KEEPCNT		6	/* Number of keepalives before death */
#define TCP_SYNCNT		7	/* Number of SYN retransmits */
#define TCP_LINGER2		8	/* Life time of orphaned FIN-WAIT-2 state */
#define TCP_DEFER_ACCEPT	9	/* Wake up listener only when data arrive */
#define TCP_WINDOW_CLAMP	10	/* Bound advertised window */
#define TCP_INFO		11	/* Information about this connection. */
#define TCP_QUICKACK		12	/* Block/reenable quick acks */
#define TCP_CONGESTION		13	/* Congestion control algorithm */
#define TCP_MD5SIG		14	/* TCP MD5 Signature (RFC2385) */
#define TCP_THIN_LINEAR_TIMEOUTS 16      /* Use linear timeouts for thin streams*/
#define TCP_THIN_DUPACK         17      /* Fast retrans. after 1 dupack */
#define TCP_USER_TIMEOUT	18	/* How long for loss retry before timeout */
#define TCP_REPAIR		19	/* TCP sock is under repair right now */
#define TCP_REPAIR_QUEUE	20
#define TCP_QUEUE_SEQ		21
#define TCP_REPAIR_OPTIONS	22
#define TCP_FASTOPEN		23	/* Enable FastOpen on listeners */
#define TCP_TIMESTAMP		24
#define TCP_NOTSENT_LOWAT	25	/* limit number of unsent bytes in write queue */
#define TCP_CC_INFO		26	/* Get Congestion Control (optional) info */
#define TCP_SAVE_SYN		27	/* Record SYN headers for new connections */
#define TCP_SAVED_SYN		28	/* Get SYN headers recorded for connection */
#define TCP_REPAIR_WINDOW	29	/* Get/set window parameters */
#define TCP_FASTOPEN_CONNECT	30	/* Attempt FastOpen with connect */
#define TCP_ULP			31	/* Attach a ULP to a TCP connection */
#define TCP_MD5SIG_EXT		32	/* TCP MD5 Signature with extensions */
#define TCP_FASTOPEN_KEY	33	/* Set the key for Fast Open (cookie) */
#define TCP_FASTOPEN_NO_COOKIE	34	/* Enable TFO without a TFO cookie */
#define TCP_ZEROCOPY_RECEIVE	35
#define TCP_INQ			36	/* Notify bytes available to read as a cmsg on read */
```





### Linux 的协议列表

[/etc/protocols](https://man7.org/linux/man-pages/man5/protocols.5.html)

```
$ cat /etc/protocols

# Internet (IP) protocols
#
# Updated from http://www.iana.org/assignments/protocol-numbers and other
# sources.
# New protocols will be added on request if they have been officially
# assigned by IANA and are not historical.
# If you need a huge list of used numbers please install the nmap package.

ip	0	IP		# internet protocol, pseudo protocol number
hopopt	0	HOPOPT		# IPv6 Hop-by-Hop Option [RFC1883]
icmp	1	ICMP		# internet control message protocol
igmp	2	IGMP		# Internet Group Management
ggp	3	GGP		# gateway-gateway protocol
ipencap	4	IP-ENCAP	# IP encapsulated in IP (officially ``IP'')
st	5	ST		# ST datagram mode
tcp	6	TCP		# transmission control protocol
```

编号：

[https://elixir.bootlin.com/linux/v5.4/source/include/linux/socket.h#L314](https://elixir.bootlin.com/linux/v5.4/source/include/linux/socket.h#L314)

```c
/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP		0
/* #define SOL_ICMP	1	No-no-no! Due to Linux :-) we cannot use SOL_ICMP=1 */
#define SOL_TCP		6 //TCP level
#define SOL_UDP		17
#define SOL_IPV6	41
#define SOL_ICMPV6	58
#define SOL_SCTP	132
#define SOL_UDPLITE	136     /* UDP-Lite (RFC 3828) */
#define SOL_RAW		255
#define SOL_IPX		256
#define SOL_AX25	257
#define SOL_ATALK	258
#define SOL_NETROM	259
#define SOL_ROSE	260
#define SOL_DECNET	261
#define	SOL_X25		262
#define SOL_PACKET	263
#define SOL_ATM		264	/* ATM layer (cell level) */
#define SOL_AAL		265	/* ATM Adaption Layer (packet level) */
#define SOL_IRDA        266
#define SOL_NETBEUI	267
#define SOL_LLC		268
#define SOL_DCCP	269
#define SOL_NETLINK	270
#define SOL_TIPC	271
#define SOL_RXRPC	272
#define SOL_PPPOL2TP	273
#define SOL_BLUETOOTH	274
#define SOL_PNPIPE	275
#define SOL_RDS		276
#define SOL_IUCV	277
#define SOL_CAIF	278
#define SOL_ALG		279
#define SOL_NFC		280
#define SOL_KCM		281
#define SOL_TLS		282
#define SOL_XDP		283
```



### Linux 的默认的参数值

需要注意的是，以下只是参数值配置。socket 是否启用 keepalive，由应用是否指定了 socket opts： `SO_KEEPALIVE` 决定。

```bash
/proc/sys/net/ipv4/:
       tcp_keepalive_intvl (integer; default: 75; since Linux 2.4)
              The number of seconds between TCP keep-alive probes.

       tcp_keepalive_probes (integer; default: 9; since Linux 2.2)
              The maximum number of TCP keep-alive probes to send before
              giving up and killing the connection if no response is
              obtained from the other end.

       tcp_keepalive_time (integer; default: 7200; since Linux 2.2)
              The number of seconds a connection needs to be idle before
              TCP begins sending out keep-alive probes.  Keep-alives are
              sent only when the SO_KEEPALIVE socket option is enabled.
              The default value is 7200 seconds (2 hours).  An idle
              connection is terminated after approximately an additional
              11 minutes (9 probes an interval of 75 seconds apart) when
              keep-alive is enabled.

              Note that underlying connection tracking mechanisms and
              application timeouts may be much shorter.
```

- [tcp man page](https://man7.org/linux/man-pages/man7/tcp.7.html#:~:text=are%20not%0A%20%20%20%20%20%20%20supported.-,/proc%20interfaces,-System%2Dwide%20TCP)
- [https://docs.kernel.org/networking/ip-sysctl.html](https://docs.kernel.org/networking/ip-sysctl.html#:~:text=tcp_keepalive_time%20%2D%20INTEGER)
- [https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/usingkeepalive.html](https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/usingkeepalive.html)


需要注意的是，以上 kernel 参数可能已经 linux network namespace 化了。即，不同容器可以独立配置。但其继承与传递关系如何，则未研究。

## 常见 Load Balancer / TLS proxy 的行为

### 自动 keepalive  load balancer 的两端

> [AWS ELB- Connection idle timeout](https://docs.aws.amazon.com/elasticloadbalancing/latest/network/network-load-balancers.html#connection-idle-timeout)
>
> For each TCP request that a client makes through a Network Load Balancer, the state of that connection is tracked. If no data is sent through the connection by either the client or target for longer than the idle timeout, the connection is closed(这里没说明，Load Balancer 会不会发出 FIN/RST). <mark>If a client or a target sends data after the idle timeout period elapses, it receives a TCP RST packet to indicate that the connection is no longer valid.</mark>
>
> We set the idle timeout value for TCP flows to 350 seconds. You can't modify this value. Clients or targets can use TCP keepalive packets to reset the idle timeout. Keepalive packets sent to maintain TLS connections can't contain data or payload.
>
> * Keepalive forward
>
> When a TLS listener receives a TCP keepalive packet from either a client or a target, the load balancer generates TCP keepalive packets and sends them to both the front-end and back-end connections every 20 seconds. You can't modify this behavior.

### 偷偷关闭连接

- [F5: Istio Ingress Gateway TCP keepalive](https://support.f5.com/csp/article/K00026550)

## 相关的 Envoy 配置资料

 - [F5: Istio Ingress Gateway TCP keepalive](https://support.f5.com/csp/article/K00026550)
 - [Istio ingress gateway TCP keepalive setting for downstream connection #28879](https://github.com/istio/istio/issues/28879)
 - [socket options - change from 'STATE_LISTENING' to 'STATE_PREBIND' #5842](https://github.com/solo-io/gloo/pull/5842)
 - [Feature request: support TCP keepalive for downstream sockets #3634](https://github.com/envoyproxy/envoy/issues/3634)
 - [AWS ELB](https://docs.aws.amazon.com/elasticloadbalancing/latest/network/network-load-balancers.html#connection-idle-timeout:~:text=deletion_protection.enabled%20attribute.-,Connection%20idle%20timeout,-For%20each%20TCP)