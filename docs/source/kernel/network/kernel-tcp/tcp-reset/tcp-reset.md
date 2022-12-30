# TCP Reset 的原因



## TCP RST and unread socket recv buffer

如果应用没有完全读完一个 socket 的 recv buffer 内的数据，就 close socket。那么 Kernel 会以 TCP RST 结束连接，而不是 FIN.

一个 Envoy 下的猜想场景是：

> Envoy 只要读完 downstream HTTP Header 就开始相关的解释和路由请求的逻辑了。如果路由到一个有问题的 upstream 连接。就开始 socket write HTTP Header 到 upstream socket 。这时写 upstream socket 失败了。 Envoy 就连 downstream 的 socket 也 close 了。 kernel 看到 Envoy 连在 buffer 中的 HTTP Body 都没读完就 close，于是发了 RST。




> - [https://stackoverflow.com/questions/54937761/socket-close2-send-rst-packet-instead-of-fin-packet](https://stackoverflow.com/questions/54937761/socket-close2-send-rst-packet-instead-of-fin-packet)



The implementation of `tcp_close` on the kernel, in the file `net/ipv4/tcp.c`.
The kernel is explained as follows:

> [https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp.c#L2389](https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/tcp.c#L2389)

```c
/* As outlined in RFC 2525, section 2.17, we send a RST here because
 * data was lost. To witness the awful effects of the old behavior of
 * always doing a FIN, run an older 2.1.x kernel or 2.0.x, start a bulk
 * GET in an FTP client, suspend the process, wait for the client to
 * advertise a zero window, then kill -9 the FTP client, wheee...
 * Note: timeout is always zero in such a case.
 */
if (unlikely(tcp_sk(sk)->repair)) {
    sk->sk_prot->disconnect(sk, 0);
} else if (data_was_unread) {
    /* Unread data was tossed, zap the connection. */
    NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONCLOSE);
    tcp_set_state(sk, TCP_CLOSE);
    tcp_send_active_reset(sk, sk->sk_allocation);
}
```



> [Known TCP Implementation Problems(rfc2525)](https://datatracker.ietf.org/doc/html/rfc2525#section-2.17)
>
> ```
> 2.17.
> 
>    Name of Problem
>       Failure to RST on close with data pending
> 
>    Classification
>       Resource management
> 
>    Description
>       When an application closes a connection in such a way that it can
>       no longer read any received data, the TCP SHOULD, per section
>       4.2.2.13 of RFC 1122, send a RST if there is any unread received
>       data, or if any new data is received. A TCP that fails to do so
>       exhibits "Failure to RST on close with data pending".
> 
>       Note that, for some TCPs, this situation can be caused by an
>       application "crashing" while a peer is sending data.
> 
>       We have observed a number of TCPs that exhibit this problem.  The
>       problem is less serious if any subsequent data sent to the now-
>       closed connection endpoint elicits a RST (see illustration below).
>       
>    Significance
>       This problem is most significant for endpoints that engage in
>       large numbers of connections, as their ability to do so will be
>       curtailed as they leak away resources.
> 
>    Implications
>       Failure to reset the connection can lead to permanently hung
>       connections, in which the remote endpoint takes no further action
>       to tear down the connection because it is waiting on the local TCP
>       to first take some action.  This is particularly the case if the
>       local TCP also allows the advertised window to go to zero, and
>       fails to tear down the connection when the remote TCP engages in
>       "persist" probes (see example below).
> 
>    Relevant RFCs
>       RFC 1122 section 4.2.2.13.  Also, 4.2.2.17 for the zero-window
>       probing discussion below.
> 
>    Trace file demonstrating it
>       Made using tcpdump.  No drop information available.
>       
> ...
> 
>    How to detect
>       The problem can often be detected by inspecting packet traces of a
>       transfer in which the receiving application terminates abnormally.
>       When doing so, there can be an ambiguity (if only looking at the
>       trace) as to whether the receiving TCP did indeed have unread data
>       that it could now no longer deliver.  To provoke this to happen,
>       it may help to suspend the receiving application so that it fails
>       to consume any data, eventually exhausting the advertised window.
>       At this point, since the advertised window is zero, we know that
>       the receiving TCP has undelivered data buffered up.  Terminating
>       the application process then should suffice to test the
>       correctness of the TCP's behavior.      
> ```





参考：

> [TCP RST: Calling close() on a socket with data in the receive queue](https://cs.baylor.edu/~donahoo/practical/CSockets/TCPRST.pdf)
>
> Consider two peers, A and B, communicating via TCP. If B closes a socket and there is any data in B’s receive queue, B sends a TCP RST to A instead of following the standard TCP closing protocol, resulting in an error return value from recv( ). 
>
> ```
> A                              B
> send()          data → 
>                 data → 
>                 data → 
> 
> recv()→ERROR    ← RST        close( ) 
> ```
>
> 
>
> When might this situation occur? Consider a simple protocol where A sends 5 prime numbers to B, and B responds with “OK” and closes the connection. If B receives a nonprime number, it assumes A is confused, responds with “ERROR”, and closes the connection. If A sends 5 numbers and the second number is not prime, then B will send “ERROR” and call close( ) with data (3 more numbers) in its receive queue. This will cause a TCP RST to be sent to A. Meanwhile, A is blocked on recv( ) awaiting word from B. The RST from B causes A’s recv( ) to return an error (-1) so A never receives the “ERROR” message from B. 
>
> 
>
> So why not just have B read all 5 prime numbers before closing? Consider the case where A sends 6 prime numbers. Here B reads the first 5 prime numbers and closes the socket with the 6th prime still in its receive queue, resulting in a RST. 





## TCP RST and SO_LINGER

```{toctree}
ref/tcprst-linger.md
```