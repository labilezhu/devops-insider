# key exchange algorithm(密钥交换算法)

## key exchange algorithm(密钥交换算法) 列举简介

### Diffie–Hellman(DH) key exchange

> [https://en.m.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange](https://en.m.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange)

Diffie-Hellman 密钥交换是一种在公共通道上安全交换加密密钥的方法，是 Ralph Merkle 构想的第一个公钥协议之一，并以 Whitfield Diffie 和 Martin Hellman 的名字命名。 DH 是在密码学领域中实现的最早的公钥交换实例之一。 由 Diffie 和 Hellman 于 1976 年出版，这是<mark>最早提出私钥和相应公钥概念的公开著作</mark>。

DH 的一种变体是： `Ephemeral Diffie-Hellman` with RSA (`DHE-RSA`)

> [https://www.thesslstore.com/blog/explaining-ssl-handshake/](https://www.thesslstore.com/blog/explaining-ssl-handshake/)
>
> There are four common variants of the DH family:
>
> - Diffie-Hellman (DH)
> - Diffie-Hellman Ephemeral (DHE)
> - Elliptic Curve Diffie-Hellman (ECDH)
> - Elliptic Curve Diffie-Hellman Ephemeral (ECDHE)

### Elliptic-curve Diffie–Hellman(ECDH)

> [https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie%E2%80%93Hellman](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie%E2%80%93Hellman)

`Elliptic-curve Diffie-Hellman (ECDH)` 是一种密钥协商协议，它允许两方，每方都有一个椭圆曲线公钥-私钥对，在不安全的通道上建立共享秘密。这个共享的秘密可以直接用作密钥，或者派生另一个密钥。 然后，该密钥或派生密钥可用于使用对称密钥密码加密后续通信。 它是使用椭圆曲线密码学的 Diffie-Hellman 协议的变体。



## key exchange algorithm 与 TLS 版本

> [https://en.m.wikipedia.org/wiki/Transport_Layer_Security#Applications_and_adoption](https://en.m.wikipedia.org/wiki/Transport_Layer_Security#Applications_and_adoption)

|                          Algorithm                           | SSL 2.0 | SSL 3.0 | TLS 1.0 | TLS 1.1 | TLS 1.2 |                           TLS 1.3                            |           Status            |
| :----------------------------------------------------------: | :-----: | :-----: | :-----: | :-----: | :-----: | :----------------------------------------------------------: | :-------------------------: |
|  [RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem))   |   Yes   |   Yes   |   Yes   |   Yes   |   Yes   |                              No                              | Defined for TLS 1.2 in RFCs |
| [DH](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) |   No    |   Yes   |   Yes   |   Yes   |   Yes   |                              No                              |                             |
| [DHE](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) ([forward secrecy](https://en.m.wikipedia.org/wiki/Transport_Layer_Security#Forward_secrecy)) |   No    |   Yes   |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
| [ECDH](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              No                              |                             |
| [ECDHE](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) (forward secrecy) |   No    |   No    |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
| [DH](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-[DSS](https://en.m.wikipedia.org/wiki/Digital_Signature_Algorithm) |   No    |   Yes   |   Yes   |   Yes   |   Yes   |                              No                              |                             |
| [DHE](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-[DSS](https://en.m.wikipedia.org/wiki/Digital_Signature_Algorithm) (forward secrecy) |   No    |   Yes   |   Yes   |   Yes   |   Yes   | No[[58\]](https://en.m.wikipedia.org/wiki/Transport_Layer_Security#cite_note-58) |                             |
| [ECDH](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-[ECDSA](https://en.m.wikipedia.org/wiki/Elliptic_Curve_DSA) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              No                              |                             |
| [ECDHE](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-[ECDSA](https://en.m.wikipedia.org/wiki/Elliptic_Curve_DSA) (forward secrecy) |   No    |   No    |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
| [ECDH](https://en.m.wikipedia.org/wiki/ECDH)-[EdDSA](https://en.m.wikipedia.org/wiki/EdDSA) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              No                              |                             |
| [ECDHE](https://en.m.wikipedia.org/wiki/ECDHE)-[EdDSA](https://en.m.wikipedia.org/wiki/EdDSA) (forward secrecy)[[59\]](https://en.m.wikipedia.org/wiki/Transport_Layer_Security#cite_note-59) |   No    |   No    |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
|        [PSK](https://en.m.wikipedia.org/wiki/TLS-PSK)        |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [PSK](https://en.m.wikipedia.org/wiki/Pre-shared_key)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [DHE](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-[PSK](https://en.m.wikipedia.org/wiki/Pre-shared_key) (forward secrecy) |   No    |   No    |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
| [ECDHE](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-[PSK](https://en.m.wikipedia.org/wiki/Pre-shared_key) (forward secrecy) |   No    |   No    |   Yes   |   Yes   |   Yes   |                             Yes                              |                             |
|        [SRP](https://en.m.wikipedia.org/wiki/TLS-SRP)        |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [SRP](https://en.m.wikipedia.org/wiki/Secure_Remote_Password_protocol)-[DSS](https://en.m.wikipedia.org/wiki/Digital_Signature_Algorithm) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [SRP](https://en.m.wikipedia.org/wiki/Secure_Remote_Password_protocol)-[RSA](https://en.m.wikipedia.org/wiki/RSA_(cryptosystem)) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [Kerberos](https://en.m.wikipedia.org/wiki/Kerberos_(protocol)) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [DH](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange)-ANON (insecure) |   No    |   Yes   |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [ECDH](https://en.m.wikipedia.org/wiki/Elliptic-curve_Diffie–Hellman)-ANON (insecure) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |                             |
| [GOST R 34.10-94/34.10-2001](https://en.m.wikipedia.org/wiki/GOST)[[60\]](https://en.m.wikipedia.org/wiki/Transport_Layer_Security#cite_note-gostlink-60) |   No    |   No    |   Yes   |   Yes   |   Yes   |                              ?                               |   Proposed in RFC drafts    |



## forward secrecy(前向保密)

> [https://en.m.wikipedia.org/wiki/Forward_secrecy](https://en.m.wikipedia.org/wiki/Forward_secrecy)

在密码学中，forward secrecy(FS/前向保密)，也称为完美前向保密 (PFS)，是特定密钥协商协议的一项功能，<mark>可确保即使会话密钥交换中使用的长期秘密被泄露，会话密钥也不会被泄露</mark>。对于 HTTPS，长期机密通常是服务器的私钥。前向保密可保护过去的会话免受未来密钥或密码的泄露。通过为用户启动的每个会话生成唯一的会话密钥，单个会话密钥的泄露不会影响除在受该特定密钥保护的特定会话中交换的数据之外的任何数据。对于前向保密来说，这本身是不够的，前向保密还要求长期的秘密妥协不会影响过去会话密钥的安全性。

前向保密保护使用通用传输层安全协议（包括 OpenSSL）的网络传输层上的数据，当其长期密钥遭到破坏时，与 Heartbleed 安全漏洞一样。如果使用前向保密，即使对手主动干预（例如通过中间人），如果长期密钥或密码在未来被泄露，过去记录的加密通信和会话将无法检索和解密中攻。

