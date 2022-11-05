# TLS


## Cipher Suite

各版本的 TLS 都支持不同的的加密算法（Cipher Suite）。同一个加密算法，也有两种命名规范（习惯）：
 - OpenSSL
 - IANA

他们的对应关系见：
> https://testssl.sh/openssl-iana.mapping.html

不同的 Library 支持的 Cipher Suite 也不同。见下面 boringssl 的例子。

## Library

```{toctree}
boringssl.md
```

