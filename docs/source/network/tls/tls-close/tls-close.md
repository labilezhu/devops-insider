# TLS Close Alert: close_notify

TLS 协议除了承载加密数据外，还要承载一些控制信号。如，流的关闭需要发送一个 `close_notify` ，有点类似 TCP 的 FIN 。

我是在一次 tcpdump 中发现这个情况的：在一个空闲的连接上， Envoy 在主动发送 TCP FIN 前，发送了一个 TLS 包。当时不知道为何有这个现象。

OpenSSL 有相关的 API:
[https://www.openssl.org/docs/man1.1.1/man3/SSL_shutdown.html](https://www.openssl.org/docs/man1.1.1/man3/SSL_shutdown.html)


## Alert protocol

> [wiki](https://en.wikipedia.org/wiki/Transport_Layer_Security#:~:text=within%20one%20record.-,Alert%20protocol,-%5Bedit%5D)

This record should normally not be sent during normal handshaking or application exchanges. However, this message can be sent at any time during the handshake and up to the closure of the session. **If this is used to signal a fatal error, the session will be closed immediately after sending this record, so this record is used to give a reason for this closure**. If the alert level is flagged as a warning, the remote can decide to close the session if it decides that the session is not reliable enough for its needs (before doing so, the remote may also send its own signal).



> [TLS 1.2 spec - 7.2.1.  Closure Alerts](https://www.rfc-editor.org/rfc/rfc5246#section-7.2.1)



```
7.2.1.  Closure Alerts

   The client and the server must share knowledge that the connection is
   ending in order to avoid a truncation attack.  Either party may
   initiate the exchange of closing messages.

   close_notify
      This message notifies the recipient that the sender will not send
      any more messages on this connection.  Note that as of TLS 1.1,
      failure to properly close a connection no longer requires that a
      session not be resumed.  This is a change from TLS 1.0 to conform
      with widespread implementation practice.

   Either party may initiate a close by sending a close_notify alert.
   Any data received after a closure alert is ignored.

   Unless some other fatal alert has been transmitted, each party is
   required to send a close_notify alert before closing the write side
   of the connection.  The other party MUST respond with a close_notify
   alert of its own and close down the connection immediately,
   discarding any pending writes.  It is not required for the initiator
   of the close to wait for the responding close_notify alert before
   closing the read side of the connection.

   If the application protocol using TLS provides that any data may be
   carried over the underlying transport after the TLS connection is
   closed, the TLS implementation must receive the responding
   close_notify alert before indicating to the application layer that
   the TLS connection has ended.  If the application protocol will not
   transfer any additional data, but will only close the underlying
   transport connection, then the implementation MAY choose to close the
   transport without waiting for the responding close_notify.  No part
   of this standard should be taken to dictate the manner in which a
   usage profile for TLS manages its data transport, including when
   connections are opened or closed.

   Note: It is assumed that closing a connection reliably delivers
   pending data before destroying the transport.
```







