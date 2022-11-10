# Resetting a TCP connection and SO_LINGER

> [https://ndeepak.com/posts/2016-10-21-tcprst/](https://ndeepak.com/posts/2016-10-21-tcprst/)



Can you quickly close a TCP connection in your program by sending a reset (“RST”) packet?

Calling `close()` usually starts an orderly shutdown, via a “FIN” packet. This means the system has to go through the full TCP shutdown sequence, where it has to get back an ACK, and a FIN from the other end also, which itself needs an ACK (called _LAST\_ACK_, quite appropriately).

So: How can you send a reset and get rid of the connection in one shot?

## Using linger option to reset

Here’s what I found on FreeBSD 8.4, based on what I see in the kernel.

1. Set `SO_LINGER` option on the socket, with a linger interval of zero.
2. Call `close()`.

In C, you just need these lines to set up the socket:

```C
struct linger sl;
sl.l_onoff = 1;		/* non-zero value enables linger option in kernel */
sl.l_linger = 0;	/* timeout interval in seconds */
setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
```

Here is the relevant kernel code that gets executed, from `netinet/tcp_usrreq.c`. The function is `tcp_disconnect()`:

```C
    /* tcp_drop() sends a RST packet on established connections */
1587         } else if ((so->so_options & SO_LINGER) && so->so_linger == 0) {
1588                 tp = tcp_drop(tp, 0);
1589                 KASSERT(tp != NULL,
1590                     ("tcp_disconnect: tcp_drop() returned NULL"));
1591         }
```

## Sample traces

I wrote a simple program to connect to a server, set the above `SO_LINGER` option with linger interval of 0, and then close. Here is the packet trace, edited to focus on TCP flags.

```plain
12.828218 IP clt.59784 > svr.http: Flags [S], seq 2974233491, ... length 0
12.828273 IP svr.http > clt.59784: Flags [S.], seq 3927686263, ack 2974233492, ... length 0
12.828312 IP clt.59784 > svr.http: Flags [.], ack 1, ... length 0
12.828324 IP clt.59784 > svr.http: Flags [R.], seq 1, ack 1, ... length 0
```

Note the abrupt close via reset (‘R’ flag).

Here is a trace _without_ `SO_LINGER` option, again, trimmed for clarity.

```plain
32.812991 IP clt.16575 > svr.http: Flags [S], seq 2769061407, ... length 0
32.813092 IP svr.http > clt.16575: Flags [S.], seq 1785234162, ack 2769061408, ... length 0
32.813110 IP clt.16575 > svr.http: Flags [.], ack 1, ... length 0
32.813786 IP clt.16575 > svr.http: Flags [F.], seq 1, ack 1, ... length 0
32.813843 IP svr.http > clt.16575: Flags [.], ack 2, ... length 0
32.814907 IP svr.http > clt.16575: Flags [F.], seq 1, ack 2, ... length 0
32.815107 IP clt.16575 > svr.http: Flags [.], ack 2, ... length 0
```

Note the extra packets, and also the FIN packets (‘F’ flag) and their acknowledgments ('.' flag).

## What is SO\_LINGER?

The remaining part of this post is about `SO_LINGER`. Let us review the documentation:

> SO\_LINGER controls the action taken when unsent messages are queued on socket and a close(2) is performed. If the socket promises reliable delivery of data and SO\_LINGER is set, the system will block the process on the close(2) attempt until it is able to transmit the data or until it decides it is unable to deliver the information (a timeout period, termed the linger interval, is specified in seconds in the setsockopt() system call when SO\_LINGER is requested).

From the man page, it seems as if the system waits only for the duration of linger interval to drain the data. But it turns out to be slightly different, as we will see. The _user application_ is blocked until the linger interval, but the OS continues to try to drain the data beyond that.

The relevant source code is in `uipc_socket.c`, function `soclose()`:

```C
689    if (so->so_options & SO_LINGER) {
               /* ... */
693            while (so->so_state & SS_ISCONNECTED) {
694                error = tsleep(&so->so_timeo,
695                               PSOCK | PCATCH, "soclos", so->so_linger * hz);
696                               if (error)
697                                   break;
698            }
699    }
```

`tsleep()` is documented in [sleep(9)](https://www.freebsd.org/cgi/man.cgi?query=msleep&apropos=0&sektion=0&manpath=FreeBSD+8.4-RELEASE&arch=default&format=html).

It is a bit tricky to exercise `SO_LINGER`, but not impossible. In fact, this is how I tried to understand the linger option:

I wrote a client program that sets linger timeout to 5 seconds, puts 90001 bytes of data to send via TCP, calls `close()`, and then exits. I also wrote a server program that accepts a connection and then goes to sleep for 30 seconds, and then reads all the remaining data. This simulates unsent data on the client at the time of invoking `close()`.

Here is the trace:

```plain
# connection open
09:41:15.940732 IP clt.44145 > svr.5000: Flags [S], seq 4078427161, win 65535, length 0
15.940780 IP svr.5000 > clt.44145: Flags [S.], seq 3774371314, ack 4078427162, win 65535, length 0
15.940797 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 0
# initial data push, server asleep but system accepts data up to socket buffer
15.940832 IP clt.44145 > svr.5000: Flags [P.], ack 1, win 8960, length 9
15.941616 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14336
15.941628 IP svr.5000 > clt.44145: Flags [.], ack 14346, win 7166, length 0
15.941684 IP clt.44145 > svr.5000: Flags [P.], ack 1, win 8960, length 1
15.942466 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14336
15.942479 IP svr.5000 > clt.44145: Flags [.], ack 28683, win 5374, length 0
15.942518 IP clt.44145 > svr.5000: Flags [P.], ack 1, win 8960, length 1
15.943301 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14336
15.943318 IP svr.5000 > clt.44145: Flags [.], ack 43020, win 3582, length 0
15.943359 IP clt.44145 > svr.5000: Flags [P.], ack 1, win 8960, length 1
15.944146 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14336
15.944163 IP svr.5000 > clt.44145: Flags [.], ack 57357, win 1790, length 0
15.944204 IP clt.44145 > svr.5000: Flags [P.], ack 1, win 8960, length 1
16.040862 IP svr.5000 > clt.44145: Flags [.], ack 57358, win 1790, length 0
# trying to drain after linger interval
21.040918 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14320
# server buffer full
21.140894 IP svr.5000 > clt.44145: Flags [.], ack 71678, win 0, length 0
# client probes after every linger interval
26.140943 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 1
26.141008 IP svr.5000 > clt.44145: Flags [.], ack 71679, win 0, length 0
31.140992 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 1
31.141056 IP svr.5000 > clt.44145: Flags [.], ack 71680, win 0, length 0
36.141022 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 1
36.141091 IP svr.5000 > clt.44145: Flags [.], ack 71681, win 0, length 0
41.141102 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 1
41.141154 IP svr.5000 > clt.44145: Flags [.], ack 71681, win 0, length 0
# server wakes up, reads buffered data
45.951247 IP svr.5000 > clt.44145: Flags [.], ack 71681, win 3601, length 0
# client drains rest of the data, closes connection
45.951317 IP clt.44145 > svr.5000: Flags [.], ack 1, win 8960, length 14336
45.951331 IP clt.44145 > svr.5000: Flags [FP.], seq 86017:90001, ack 1, win 8960, length 3984
# server acks, opens window
45.951345 IP svr.5000 > clt.44145: Flags [.], ack 90002, win 4244, length 0
45.951419 IP svr.5000 > clt.44145: Flags [.], ack 90002, win 7846, length 0
46.008793 IP svr.5000 > clt.44145: Flags [.], ack 90002, win 8960, length 0
# socket already closed at client, so tear down connection with a reset
46.008835 IP clt.44145 > svr.5000: Flags [R], seq 4078517163, win 0, length 0
```

On the client, the `close()` call returns the following `EWOULDBLOCK` as expected, but note how the program is blocked until 5 seconds later:

```plain
closing, time now: Sat Oct 22 09:41:15 PDT 2016
close: Resource temporarily unavailable
exiting, time now: Sat Oct 22 09:41:20 PDT 2016
```

The socket wakes up every 5 seconds (our linger interval), trying to drain remaining data. Note that the server buffers are exhausted as evident by the zero window advertised. Finally, the server reads up the remaining data and the connection is closed with a reset because the application has exited.

Even though the server application is asleep, the server system is able to accept 71678 bytes of data in its receive buffers before advertising empty space. It is the remaining unsent data on the client that triggers the ‘linger’ option.

I tried a few other combinations with my test programs to see how `SO_LINGER` behaves:

- If client program is still running, then the OS keeps the connection open and waits for the server to send a FIN from its end.
- If the server wakes up before the linger interval, then the program returns before the linger interval.
- If this is a non-blocking socket, `close()` does not block, but the system still tries to drain data.

I’ve posted the test programs: [server](https://ndeepak.com/files/linsvr.c), [client](https://ndeepak.com/files/linclt.c).

## To linger or not, that is the question

So, what does this all mean to the BSD network programmer?

- Use `SO_LINGER` with linger interval of zero to send a reset immediately, whether your socket is blocking or non-blocking.
- Use `SO_LINGER` with non-zero linger interval to block your program until that time to send the data. It can wake up before that if it is able to drain the data. If your socket is non-blocking, it returns immediately but the OS will still try to drain the data.
- Make sure you set `l_onoff` field in the `linger` structure to a non-zero value, else none of this applies.

## SO\_LINGER on Linux

I had to tweak the client program a bit to simulate this on Linux. I used Ubuntu 16.04, running kernel 4.8.3. I kept calling non-blocking `send(..., MSG_DONTWAIT)` on the client until I got an `EWOULDBLOCK` error. The `tcpdump` utility needed a bigger buffer (`-B` option) so as not to drop packets.

As far as the user program is concerned, the behavior is similar:

- Zero linger interval resulted in a TCP reset packet
- Client program was blocked until the sleep interval or data was drained
- After that, the OS continued to try to drain the data
- When it was done, it sent a FIN to initiate a close

But I saw a few differences:

- The `close()` call did not return `EWOULDBLOCK` like BSD, when I used regular (i.e., blocking) socket.
- The `close()` call blocked the program for the duration of linger interval, even on a non-blocking socket. This looks bad!
- The probes to drain data became less frequent over time (‘exponential backoff’):

```plain
# ... many packets omitted ...
21:38:45.228236 IP clt.5000 > clt.35512: Flags [.], ack 938815, win 141, ... length 0
21:38:45.464879 IP clt.35512 > clt.5000: Flags [P.], seq 938815:956863, ack 1, win 342, ... length 18048
# first zero window from server
21:38:45.464910 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 240ms interval
21:38:45.704875 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:38:45.704905 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 450ms interval
21:38:46.158240 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
# 880ms interval
21:38:47.038229 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:38:47.038258 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 1.89s interval
21:38:48.931618 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:38:48.931670 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 3.62s interval
21:38:52.558238 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:38:52.558278 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 7.04s interval
21:38:59.598276 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:38:59.598322 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 14.7s interval
21:39:14.318292 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:39:14.318357 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 29.0s interval
21:39:43.331579 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:39:43.331630 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 56.3s interval
21:40:39.651571 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:40:39.651616 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# 116s interval
21:42:35.704920 IP clt.35512 > clt.5000: Flags [.], ack 1, win 342, ... length 0
21:42:35.704995 IP clt.5000 > clt.35512: Flags [.], ack 956863, win 0, ... length 0
# ... many more packets omitted ...
# server woke up and read data
21:43:45.074313 IP clt.5000 > clt.35512: Flags [.], ack 2444924, win 8527, ... length 0
# drain almost complete ...
21:43:45.074351 IP clt.35512 > clt.5000: Flags [.], seq 3296203:3361686, ack 1, win 342, ... length 65483
# complete, and the FIN packet
21:43:45.074379 IP clt.35512 > clt.5000: Flags [FP.], seq 3361686:3427169, ack 1, win 342, ... length 65483
```

The `netstat -t` command showed the socket in _FIN\_WAIT\_1_ state while this drain was in progress. Also note the non-empty send queue.

```plain
$ netstat -t
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State
tcp        0 2470307 localhost:35512         localhost:5000          FIN_WAIT1
tcp   956862      0 localhost:5000          localhost:35512         ESTABLISHED
```

This seems queer, because _FIN\_WAIT\_1_ implies that the FIN packet is already sent out. But this may be a reflection of the socket state rather than the state of the underlying TCP connection. It did move to _FIN\_WAIT\_2_ after the FIN was acknowledged by the server when it woke up and the server did not bother to call a `close()` of its own.

```plain
$ netstat -t
Active Internet connections (w/o servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State
tcp        0      0 localhost:35512         localhost:5000          FIN_WAIT2
tcp        0      0 localhost:5000          localhost:35512         CLOSE_WAIT
```

## Linux implementation

I had a cursory look at the kernel source code. It is in `net/ipv4/tcp.c`, function `tcp_close()`:

```C
2069         } else if (sock_flag(sk, SOCK_LINGER) && !sk->sk_lingertime) {
2070                 /* Check zero linger _after_ checking for unread data. */
2071                 sk->sk_prot->disconnect(sk, 0);
2072                 NET_INC_STATS_USER(sock_net(sk), LINUX_MIB_TCPABORTONDATA);
```

If a timeout is set, it is processed in `net/ipv4/af_inet.c`, function `inet_release()`. The `close()` function takes in a timeout parameter.

```C
404                 /* If linger is set, we don't return until the close
405                  * is complete.  Otherwise we return immediately. The
406                  * actually closing is done the same either way.
407                  *
408                  * If the close is due to the process exiting, we never
409                  * linger..
410                  */
411                 timeout = 0;
412                 if (sock_flag(sk, SOCK_LINGER) &&
413                     !(current->flags & PF_EXITING))
414                         timeout = sk->sk_lingertime;
415                 sock->sk = NULL;
416                 sk->sk_prot->close(sk, timeout);
```

## To linger or not, Linux edition

So, what does this mean to the Linux network programmer?

- If you’re using non-blocking sockets, note that calling `close()` may block the program
- Otherwise, the advice previously on BSD still holds good
