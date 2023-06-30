
## run

### mock http server

#### build images

```
cd $HOME/devops-insider/docs/source/observability/log/fluentbit/test-http-output

export DOMAIN_NAME=localhost
export DAYS_VALID=100
openssl \
    req -x509 \
    -nodes \
    -subj "/CN=${DOMAIN_NAME}}" \
    -addext "subjectAltName=DNS:${DOMAIN_NAME}" \
    -days ${DAYS_VALID} \
    -newkey rsa:2048 -keyout ./self-signed.key \
    -out ./self-signed.crt

docker build . -f nginx.Dockerfile -t nginx-self-signed


```

#### run mock

```
docker network create httpbin

docker stop httpbin
docker run -it --rm -d -p 8081:80 --network=httpbin --name=httpbin --hostname=httpbin kennethreitz/httpbin
curl -X POST "http://localhost:8081/post" -H "accept: application/json"

docker stop nginx
docker rm nginx
docker run -d --rm -p 444:443 --network=httpbin --name=nginx --hostname=nginx \
-v $HOME/devops-insider/docs/source/observability/log/fluentbit/test-http-output/etc/nginx/conf.d:/etc/nginx/conf.d \
--link httpbin nginx-self-signed


docker exec nginx curl -v -k -X POST "http://httpbin/post"

docker exec nginx curl -v -k -X POST "https://localhost/post" -H "accept: application/json"

curl -v -k -X POST "http://localhost:8080/post" -H "accept: application/json"


docker logs -f httpbin
docker logs -f nginx 

curl -v -k -X POST "https://localhost:444/post" -H "accept: application/json"

```

### run fluent-bit

```
cd ${HOME}/opensource/fluent-bit/build/bin
${HOME}/opensource/fluent-bit/build/bin/fluent-bit -v --config=${HOME}/devops-insider/docs/source/observability/log/fluentbit/test-http-output/fluentbit-http.conf
```

## debug


c->u_conn->u->net->io_timeout


```
tls_net_write@0x000055555560a33d (/home/mark/opensource/fluent-bit/src/tls/openssl.c:442)
flb_tls_net_write_async@0x000055555560abd4 (/home/mark/opensource/fluent-bit/src/tls/flb_tls.c:278)
flb_io_net_write@0x0000555555618dba (/home/mark/opensource/fluent-bit/src/flb_io.c:421)
flb_http_do@0x000055555561b5c8 (/home/mark/opensource/fluent-bit/src/flb_http_client.c:1183)
http_post@0x000055555569571c (/home/mark/opensource/fluent-bit/plugins/out_http/http.c:248)
cb_http_flush@0x0000555555696aeb (/home/mark/opensource/fluent-bit/plugins/out_http/http.c:594)
output_pre_cb_flush@0x00005555555dbb0e (/home/mark/opensource/fluent-bit/include/fluent-bit/flb_output.h:517)
co_init@0x000055555598104b (/home/mark/opensource/fluent-bit/lib/monkey/deps/flb_libco/amd64.c:117)
??@0x0000000000000000 (Unknown Source:0)
```


```
tls_net_read@0x000055555560a19d (/home/mark/opensource/fluent-bit/src/tls/openssl.c:405)
flb_tls_net_read_async@0x000055555560a9fc (/home/mark/opensource/fluent-bit/src/tls/flb_tls.c:203)
flb_io_net_read@0x000055555561902f (/home/mark/opensource/fluent-bit/src/flb_io.c:465)
flb_http_do@0x000055555561b78e (/home/mark/opensource/fluent-bit/src/flb_http_client.c:1233)
http_post@0x000055555569571c (/home/mark/opensource/fluent-bit/plugins/out_http/http.c:248)
cb_http_flush@0x0000555555696aeb (/home/mark/opensource/fluent-bit/plugins/out_http/http.c:594)
output_pre_cb_flush@0x00005555555dbb0e (/home/mark/opensource/fluent-bit/include/fluent-bit/flb_output.h:517)
co_init@0x000055555598104b (/home/mark/opensource/fluent-bit/lib/monkey/deps/flb_libco/amd64.c:117)
??@0x0000000000000000 (Unknown Source:0)
```



### breakpoints
```
catch syscall read
Catchpoint 16 (syscall 'read' [0])

catch syscall readv
catch syscall write


condition <breakpoint_number> condition

catch syscall
Catchpoint 17 (any syscall)
condition 17 ($_thread == 4 || $_thread == 5)

 if $_thread == 1

```


## debug symbol

```
sudo apt install libssl-dev
```