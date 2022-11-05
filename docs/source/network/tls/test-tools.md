# 测试工具

## TESTSSL

一个检测 TLS 服务端支持的 `加密算法(Cipher Suite)` 的工具：

testssl.sh is a free command line tool which checks a server's service on any port for the support of TLS/SSL ciphers, protocols as well as cryptographic flaws and much more.

The output rates findings by color (screen) or severity (file output) so that you are able to tell whether something is good or bad. The (screen) output has several sections in which classes of checks are being performed. To ease readability on the screen it aligns and indents the output properly.

Only you see the result. You also can use it internally on your LAN. Except DNS lookups or unless you instruct testssl.sh to check for revocation of certificates it doesn't use any other hosts or even third parties for any test.

> [https://testssl.sh/doc/testssl.1.html](https://testssl.sh/doc/testssl.1.html)

