# Dump TLS(TLS 抓包)



## SSLKEYLOGFILE

> [https://tshark.dev/export/export_tls/](https://tshark.dev/export/export_tls/)



## Using tshark to Decrypt SSL/TLS Packets

> [Using tshark to Decrypt SSL/TLS Packets](https://minnmyatsoe.com/2016/01/26/using-tshark-to-decrypt-ssl-tls-packets/)

```bash
# [1] create RSA cert and key pair
openssl req -new -x509 -out server.crt -nodes -keyout server.pem -subj /CN=localhost

# [2] run the server using the above
openssl s_server -www -cipher AES256-SHA -key server.pem -cert server.crt -accept 4443

# [3] from another console session, start capturing the traffic, on loopback interface
# (you will need to change lo0 to the relevant interface on your system.
tshark -s0 -w ssltest.pcap -i lo0

# [4] generate traffic from another console
curl -vk https://localhost:4443

# [5] Ctrl+C on the tshark command at [3], and stop the openssl server at [2]
```

At this point, we should have the file called ssltest.pcap from **tshark**, and server.crt/server.pem from **openssl** commands.

Next, we are going to read the pcap file and decode the traffic.

```bash
# [1] it shows the encrypted traffic
tshark -r ssltest.pcap

# [2] for details of the packets
tshark -r ssltest.pcap -V

# [3] for decrypted data; ssl.keys_list points to the RSA key
# added -x for hex dump
# At the output you should see the message in packet detail:
#  >>> Decrypted SSL record (16 bytes):
# And the decrypted data:
# >>> Hypertext Transfer Protocol
# >>>    GET / HTTP/1.1\r\n
tshark -r ssltest.pcap -V -x -o "ssl.debug_file:ssldebug.log" -o "ssl.desegment_ssl_records: TRUE" -o "ssl.desegment_ssl_application_data: TRUE" -o "ssl.keys_list:127.0.0.1,4443,http,server.pem"

# [4] inspecting ssldebug.log output from [3]
# You should see the following messeage near the top of the file:
#   >>> ssl_init private key file server.pem successfully loaded.
cat ssldebug.log
```
