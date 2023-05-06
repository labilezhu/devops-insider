# TCP MSS



## Connection Info Introduction

### example output

```
$ ss -taoipnm

ESTAB 0      0               192.164.1.179:55124           10.2.16.235:9042  users:(("envoy",pid=81281,fd=50))
         ts sack cubic wscale:9,7 rto:204 rtt:0.689/0.065 ato:40 mss:1448 pmtu:9000 rcvmss:610 advmss:8948 cwnd:10 bytes_sent:3639 bytes_retrans:229974096 bytes_acked:3640 bytes_received:18364 segs_out:319 segs_in:163 data_segs_out:159 data_segs_in:159 send 168.1Mbps lastsnd:16960 las
trcv:16960 lastack:16960 pacing_rate 336.2Mbps delivery_rate 72.4Mbps delivered:160 app_limited busy:84ms retrans:0/25813 rcv_rtt:1 rcv_space:62720 rcv_ssthresh:56588 minrtt:0.16
```



### MTU/MSS

#### mss

current effective sending MSS.

>https://github.com/CumulusNetworks/iproute2/blob/6335c5ff67202cf5b39eb929e2a0a5bb133627ba/misc/ss.c#L2206
>
>```c
>s.mss		 = info->tcpi_snd_mss
>```
>
>https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp.c#L3258
>
>```c
>	info->tcpi_snd_mss = tp->mss_cache;
>```
>
>https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_output.c#L1576
>
>```c
>/*
>tp->mss_cache is current effective sending mss, including
>all tcp options except for SACKs. It is evaluated,
>taking into account current pmtu, but never exceeds
>tp->rx_opt.mss_clamp.
>...
>*/
>unsigned int tcp_sync_mss(struct sock *sk, u32 pmtu)
>{
>...
>	tp->mss_cache = mss_now;
>
>	return mss_now;
>}
>```
>
>



#### advmss

Advertised MSS by the host when conection started(in SYN packet).

> https://elixir.bootlin.com/linux/v5.4/source/include/linux/tcp.h#L217
>
> 

#### pmtu

path MTU value.  I (Mark Zhu) guess it is the result of Path MTU Discovery .

> https://github.com/shemminger/iproute2/blob/f8decf82af07591833f89004e9b72cc39c1b5c52/misc/ss.c#L3075
>
> ```c
> 		s.pmtu		 = info->tcpi_pmtu;
> ```
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp.c#L3272
>
> ```c
> 	info->tcpi_pmtu = icsk->icsk_pmtu_cookie;
> ```
>
> 
>
> https://elixir.bootlin.com/linux/v5.4/source/include/net/inet_connection_sock.h#L96
>
> ```c
> //@icsk_pmtu_cookie	   Last pmtu seen by socket
> struct inet_connection_sock {
> 	...
> 	__u32			  icsk_pmtu_cookie;
> ```
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_output.c#L1573
>
> ```c
> unsigned int tcp_sync_mss(struct sock *sk, u32 pmtu)
> {
>  /* And store cached results */
> 	icsk->icsk_pmtu_cookie = pmtu;
> ```
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_input.c#L2587
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_ipv4.c#L362
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_timer.c#L161

#### rcvmss

MSS used for delayed ACK decisions

> https://elixir.bootlin.com/linux/v5.4/source/include/net/inet_connection_sock.h#L122
>
> ```c
> 		__u16		  rcv_mss;	 /* MSS used for delayed ACK decisions	   */
> ```
>
> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp_input.c#L502
>
> ```c
> /* Initialize RCV_MSS value.
>  * RCV_MSS is an our guess about MSS used by the peer.
>  * We haven't any direct information about the MSS.
>  * It's better to underestimate the RCV_MSS rather than overestimate.
>  * Overestimations make us ACKing less frequently than needed.
>  * Underestimations are more easy to detect and fix by tcp_measure_rcv_mss().
>  */
> void tcp_initialize_rcv_mss(struct sock *sk)
> {
> 	const struct tcp_sock *tp = tcp_sk(sk);
> 	unsigned int hint = min_t(unsigned int, tp->advmss, tp->mss_cache);
> 
> 	hint = min(hint, tp->rcv_wnd / 2);
> 	hint = min(hint, TCP_MSS_DEFAULT);
> 	hint = max(hint, TCP_MIN_MSS);
> 
> 	inet_csk(sk)->icsk_ack.rcv_mss = hint;
> }
> ```







## MSS Setting



```{toctree}
tcp-mss-setting.md
```





## Ref.







