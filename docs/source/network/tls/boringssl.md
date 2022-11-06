# boringssl

## Cipher Suite

各版本的 TLS 都支持不同的的加密算法（Cipher Suite）。boringssl 支持的算法列表与配置方法见：
> [https://commondatastorage.googleapis.com/chromium-boringssl-docs/ssl.h.html#Cipher-suite-configuration](https://commondatastorage.googleapis.com/chromium-boringssl-docs/ssl.h.html#Cipher-suite-configuration)


> [Intent to Remove: DHE-based ciphers](https://groups.google.com/a/chromium.org/g/blink-dev/c/ShRaCsYx4lk/m/46rD81AsBwAJ)

## Envoy 相关的 Github Issues
 - [Unable to add particular ciphers (Eg. DHE-RSA-AES128-GCM-SHA256) #8848](https://github.com/envoyproxy/envoy/issues/8848)
 

## 错误码列表

> [https://github.com/google/boringssl/blob/master/crypto/err/ssl.errordata](https://github.com/google/boringssl/blob/master/crypto/err/ssl.errordata)

