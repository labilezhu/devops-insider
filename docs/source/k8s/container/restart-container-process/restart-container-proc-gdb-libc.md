# k8s 中重启容器主进程

## setup target e.g pod

```bash
docker pull tomcat:9.0.76-jre17

docker tag tomcat:9.0.76-jre17 localhost:5000/tomcat:9.0.76-jre17

docker push localhost:5000/tomcat:9.0.76-jre17
```



```yaml
kubectl delete StatefulSet tomcat-worknode5

kubectl apply -f - <<"EOF"

apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: tomcat-worknode5
  labels:
    app: tomcat-worknode5
spec:
  replicas: 1
  selector:
    matchLabels:
      app: tomcat-worknode5
  template:
    metadata:
      labels:
          app.kubernetes.io/name: tomcat-worknode5
          app: tomcat-worknode5
      annotations:
        sidecar.istio.io/inject: "false"
    spec:
      restartPolicy: Always
      imagePullSecrets:
      - name: docker-registry-key
      containers:
      - name: main-app
        image: 192.168.122.1:5000/tomcat:9.0.76-jre17
        imagePullPolicy: IfNotPresent
        ports:
        - containerPort: 8080
          protocol: TCP
          name: http      
        - containerPort: 8070
          protocol: TCP
          name: http-m   
        - containerPort: 8079
          protocol: TCP
          name: grpc  
          
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: "kubernetes.io/hostname"
                operator: In
                values:
                - "worknode5" 

EOF
```

## inspect process

```bash
ssh labile@192.168.122.55

export POD="tomcat-worknode5-0"
PIDS=$(pgrep java)
while IFS= read -r TPID; do
    HN=$(sudo nsenter -u -t $TPID hostname)
    if [[ "$HN" = "$POD" ]]; then # space between = is important
        sudo nsenter -u -t $TPID hostname
        export POD_PID=$TPID
    fi
done <<< "$PIDS"
echo $POD_PID
export PID=$POD_PID

➜  ~ ps -f $PID | cat
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       38123   37929  2 07:06 ?        Ssl    0:04 /opt/java/openjdk/bin/java -Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties -Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager -Djdk.tls.ephemeralDHKeySize=2048 -Djava.protocol.handler.pkgs=org.apache.catalina.webresources -Dorg.apache.catalina.security.SecurityListener.UMASK=0027 -Dignore.endorsed.dirs= -classpath /usr/local/tomcat/bin/bootstrap.jar:/usr/local/tomcat/bin/tomcat-juli.jar -Dcatalina.base=/usr/local/tomcat -Dcatalina.home=/usr/local/tomcat -Djava.io.tmpdir=/usr/local/tomcat/temp org.apache.catalina.startup.Bootstrap start


➜  ~ ps -ef --forest | grep -B2 $PID
root       37929       1  0 07:06 ?        00:00:00 /usr/bin/containerd-shim-runc-v2 -namespace k8s.io -id 9a43e5d86f29e64eb67ccf2ef19d442e4a06af69def716e181fa52694ec9f43b -address /run/containerd/containerd.sock
65535      37963   37929  0 07:06 ?        00:00:00  \_ /pause
root       38123   37929  1 07:06 ?        00:00:05  \_ /opt/java/openjdk/bin/java -Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties -Djava.util.logging.manager=org.apac....local/tomcat -Djava.io.tmpdir=/usr/local/tomcat/temp org.apache.catalina.startup.Bootstrap start


➜  ~ sudo ls -l /proc/$PID/fd
total 0
lrwx------ 1 root root 64 Jun 18 07:06 0 -> /dev/null
l-wx------ 1 root root 64 Jun 18 07:06 1 -> 'pipe:[181879]'
l-wx------ 1 root root 64 Jun 18 07:07 10 -> /usr/local/tomcat/logs/host-manager.2023-06-18.log
...
lr-x------ 1 root root 64 Jun 18 07:07 19 -> /usr/local/tomcat/lib/tomcat-i18n-es.jar
l-wx------ 1 root root 64 Jun 18 07:06 2 -> 'pipe:[181880]'
lr-x------ 1 root root 64 Jun 18 07:07 20 -> /usr/local/tomcat/lib/tomcat-i18n-cs.jar
...
lrwx------ 1 root root 64 Jun 18 07:07 43 -> 'socket:[182049]'
lrwx------ 1 root root 64 Jun 18 07:07 44 -> 'socket:[182552]'
l-wx------ 1 root root 64 Jun 18 07:07 45 -> /usr/local/tomcat/logs/localhost_access_log.2023-06-18.txt
lrwx------ 1 root root 64 Jun 18 07:07 46 -> 'anon_inode:[eventpoll]'
lrwx------ 1 root root 64 Jun 18 07:07 47 -> 'anon_inode:[eventfd]'
lrwx------ 1 root root 64 Jun 18 07:07 49 -> 'socket:[182059]'
...
l-wx------ 1 root root 64 Jun 18 07:07 9 -> /usr/local/tomcat/logs/manager.2023-06-18.log


➜  ~ sudo strings /proc/$PID/environ 
KUBERNETES_SERVICE_PORT_HTTPS=443
KUBERNETES_SERVICE_PORT=443
HOSTNAME=tomcat-worknode5-0
LANGUAGE=en_US:en
JAVA_HOME=/opt/java/openjdk
GPG_KEYS=48F8E69F6390C9F25CFEDCD268248959359E722B A9C5DF4D22E99998D9875A5110C01C5A2F6059E7 DCFD35E0BF8CA7344752DE8B6FB21E8933C60243
HTTPBIN_PORT_80_TCP_PORT=80
PWD=/usr/local/tomcat
TOMCAT_SHA512=028163cbe15367f0ab60e086b0ebc8d774e62d126d82ae9152f863d4680e280b11c9503e3b51ee7089ca9bea1bfa5b535b244a727a3021e5fa72dd7e9569af9a
TOMCAT_MAJOR=9
HOME=/root
LANG=en_US.UTF-8
KUBERNETES_PORT_443_TCP=tcp://10.96.0.1:443
HTTPBIN_SERVICE_PORT=80
TOMCAT_NATIVE_LIBDIR=/usr/local/tomcat/native-jni-lib
CATALINA_HOME=/usr/local/tomcat
SHLVL=0
KUBERNETES_PORT_443_TCP_PROTO=tcp
JDK_JAVA_OPTIONS= --add-opens=java.base/java.lang=ALL-UNNAMED --add-opens=java.base/java.io=ALL-UNNAMED --add-opens=java.base/java.util=ALL-UNNAMED --add-opens=java.base/java.util.concurrent=ALL-UNNAMED --add-opens=java.rmi/sun.rmi.transport=ALL-UNNAMED
FORTIO_SERVER_SERVICE_PORT_HTTP=8080
KUBERNETES_PORT_443_TCP_ADDR=10.96.0.1
LD_LIBRARY_PATH=/usr/local/tomcat/native-jni-lib
KUBERNETES_SERVICE_HOST=10.96.0.1
LC_ALL=en_US.UTF-8
KUBERNETES_PORT=tcp://10.96.0.1:443
KUBERNETES_PORT_443_TCP_PORT=443
PATH=/usr/local/tomcat/bin:/opt/java/openjdk/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
TOMCAT_VERSION=9.0.76
JAVA_VERSION=jdk-17.0.7+7


➜  ~ sudo nsenter -n -t $PID ss -tpna | grep $PID
LISTEN   0        1           [::ffff:127.0.0.1]:8005     *:*   users:(("java",pid=38123,fd=49))   
LISTEN   0        100                          *:8080     *:*   users:(("java",pid=38123,fd=43)) 


```

## gdb

```bash
sudo gdb -p $PID
...
0x00007ffff7dea197 in ?? () from target:/lib/x86_64-linux-gnu/libc.so.6

➜  ~ ps -f $PID
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       38123   37929  0 07:06 ?        tsl    0:06 /opt/java/openjdk/bin/java -Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties -Dj


```

### close fd
```bash

(gdb) set $max=49
(gdb) set $current=3
(gdb) while ($current <= $max)
 > p (int) close($current++)
 >end
```

```bash
➜  ~ sudo ls -l /proc/$PID/fd                    
total 0
lrwx------ 1 root root 64 Jun 18 07:06 0 -> /dev/null
l-wx------ 1 root root 64 Jun 18 07:06 1 -> 'pipe:[181879]'
l-wx------ 1 root root 64 Jun 18 07:06 2 -> 'pipe:[181880]'
```

### exec
```bash
call execl("/usr/bin/cat","cat",0)

p (int) execl("/opt/java/openjdk/bin/java", "java", "-DgdbRestarted=true", "-Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties", "-Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager", "-Djdk.tls.ephemeralDHKeySize=2048", "-Djava.protocol.handler.pkgs=org.apache.catalina.webresources", "-Dorg.apache.catalina.security.SecurityListener.UMASK=0027", "-Dignore.endorsed.dirs=", "-classpath", "/usr/local/tomcat/bin/bootstrap.jar:/usr/local/tomcat/bin/tomcat-juli.jar", "-Dcatalina.base=/usr/local/tomcat", "-Dcatalina.home=/usr/local/tomcat", "-Djava.io.tmpdir=/usr/local/tomcat/temp", "org.apache.catalina.startup.Bootstrap", "start",0)
```


### check
```bash
➜  ~ ps -f $PID | cat
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       38123   37929  0 07:06 ?        ts     0:07 java -DgdbRestarted=true -Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties -Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager -Djdk.tls.ephemeralDHKeySize=2048 -Djava.protocol.handler.pkgs=org.apache.catalina.webresources -Dorg.apache.catalina.security.SecurityListener.UMASK=0027 -Dignore.endorsed.dirs= -classpath /usr/local/tomcat/bin/bootstrap.jar:/usr/local/tomcat/bin/tomcat-juli.jar -Dcatalina.base=/usr/local/tomcat -Dcatalina.home=/usr/local/tomcat -Djava.io.tmpdir=/usr/local/tomcat/temp org.apache.catalina.startup.Bootstrap start
```

### detach
```
(gdb) detach
```

### check after restarted

```
➜  ~ ps -f $PID | cat
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       38123   37929  0 07:06 ?        Ssl    0:10 java -DgdbRestarted=true -Djava.util.logging.config.file=/usr/local/tomcat/conf/logging.properties -Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager -Djdk.tls.ephemeralDHKeySize=2048 -Djava.protocol.handler.pkgs=org.apache.catalina.webresources -Dorg.apache.catalina.security.SecurityListener.UMASK=0027 -Dignore.endorsed.dirs= -classpath /usr/local/tomcat/bin/bootstrap.jar:/usr/local/tomcat/bin/tomcat-juli.jar -Dcatalina.base=/usr/local/tomcat -Dcatalina.home=/usr/local/tomcat -Djava.io.tmpdir=/usr/local/tomcat/temp org.apache.catalina.startup.Bootstrap start

➜  ~ sudo ls -l /proc/$PID/fd  
total 0
lrwx------ 1 root root 64 Jun 18 07:46 0 -> /dev/null
l-wx------ 1 root root 64 Jun 18 07:46 1 -> 'pipe:[181879]'
l-wx------ 1 root root 64 Jun 18 07:50 10 -> /usr/local/tomcat/logs/host-manager.2023-06-18.log
lr-x------ 1 root root 64 Jun 18 07:50 11 -> /usr/local/tomcat/lib/jaspic-api.jar
...
lr-x------ 1 root root 64 Jun 18 07:50 17 -> /usr/local/tomcat/lib/el-api.jar
lr-x------ 1 root root 64 Jun 18 07:50 18 -> /usr/local/tomcat/lib/ecj-4.20.jar
lr-x------ 1 root root 64 Jun 18 07:50 19 -> /usr/local/tomcat/lib/tomcat-i18n-es.jar
l-wx------ 1 root root 64 Jun 18 07:46 2 -> 'pipe:[181880]'
lr-x------ 1 root root 64 Jun 18 07:50 20 -> /usr/local/tomcat/lib/tomcat-i18n-cs.jar
lr-x------ 1 root root 64 Jun 18 07:50 21 -> /usr/local/tomcat/lib/annotations-api.jar
lr-x------ 1 root root 64 Jun 18 07:50 22 -> /usr/local/tomcat/lib/servlet-api.jar
...
lr-x------ 1 root root 64 Jun 18 07:50 42 -> /usr/local/tomcat/lib/tomcat-i18n-ru.jar
lrwx------ 1 root root 64 Jun 18 07:50 43 -> 'socket:[282010]'
lrwx------ 1 root root 64 Jun 18 07:50 44 -> 'socket:[281414]'
l-wx------ 1 root root 64 Jun 18 07:50 45 -> /usr/local/tomcat/logs/localhost_access_log.2023-06-18.txt
lrwx------ 1 root root 64 Jun 18 07:50 46 -> 'anon_inode:[eventpoll]'
lrwx------ 1 root root 64 Jun 18 07:50 47 -> 'anon_inode:[eventfd]'
lrwx------ 1 root root 64 Jun 18 07:50 49 -> 'socket:[282022]'
lr-x------ 1 root root 64 Jun 18 07:50 5 -> /usr/local/tomcat/bin/bootstrap.jar
lr-x------ 1 root root 64 Jun 18 07:50 6 -> /usr/local/tomcat/bin/commons-daemon.jar
l-wx------ 1 root root 64 Jun 18 07:50 7 -> /usr/local/tomcat/logs/catalina.2023-06-18.log
l-wx------ 1 root root 64 Jun 18 07:50 8 -> /usr/local/tomcat/logs/localhost.2023-06-18.log
l-wx------ 1 root root 64 Jun 18 07:50 9 -> /usr/local/tomcat/logs/manager.2023-06-18.log

➜  ~ sudo nsenter -n -t $PID ss -tpna | grep $PID
LISTEN   0        1           [::ffff:127.0.0.1]:8005    *:*       users:(("java",pid=38123,fd=49)) 
LISTEN   0        100                          *:8080    *:*       users:(("java",pid=38123,fd=43))

```



```bash
kubectl logs tomcat-worknode5-0 | less

NOTE: Picked up JDK_JAVA_OPTIONS:  --add-opens=java.base/java.lang=ALL-UNNAMED --add-opens=java.base/java.io=ALL-UNNAMED --add-opens=java.base/java.util=ALL-UNNAMED --add-opens=java.base/java.util.concurrent=ALL-UNNAMED --add-opens=java.rmi/sun.rmi.transport=ALL-UNNAMED
18-Jun-2023 07:06:57.361 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server version name:   Apache Tomcat/9.0.76
18-Jun-2023 07:06:57.388 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server built:          Jun 5 2023 07:17:04 UTC
18-Jun-2023 07:06:57.389 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server version number: 9.0.76.0
18-Jun-2023 07:06:57.389 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log OS Name:               Linux
18-Jun-2023 07:06:57.390 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log OS Version:            5.4.0-152-generic
18-Jun-2023 07:06:57.390 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Architecture:          amd64
18-Jun-2023 07:06:57.390 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Java Home:             /opt/java/openjdk
18-Jun-2023 07:06:57.390 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log JVM Version:           17.0.7+7
...
18-Jun-2023 07:06:59.137 INFO [main] org.apache.coyote.AbstractProtocol.init Initializing ProtocolHandler ["http-nio-8080"]
18-Jun-2023 07:06:59.354 INFO [main] org.apache.catalina.startup.Catalina.load Server initialization in [2996] milliseconds
18-Jun-2023 07:06:59.688 INFO [main] org.apache.catalina.core.StandardService.startInternal Starting service [Catalina]
18-Jun-2023 07:06:59.689 INFO [main] org.apache.catalina.core.StandardEngine.startInternal Starting Servlet engine: [Apache Tomcat/9.0.76]
18-Jun-2023 07:06:59.771 INFO [main] org.apache.coyote.AbstractProtocol.start Starting ProtocolHandler ["http-nio-8080"]
18-Jun-2023 07:06:59.845 INFO [main] org.apache.catalina.startup.Catalina.start Server startup in [490] milliseconds


---------------------------

NOTE: Picked up JDK_JAVA_OPTIONS:  --add-opens=java.base/java.lang=ALL-UNNAMED --add-opens=java.base/java.io=ALL-UNNAMED --add-opens=java.base/java.util=ALL-UNNAMED --add-opens=java.base/java.util.concurrent=ALL-UNNAMED --add-opens=java.rmi/sun.rmi.transport=ALL-UNNAMED
18-Jun-2023 07:49:57.887 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server version name:   Apache Tomcat/9.0.76
18-Jun-2023 07:49:57.894 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server built:          Jun 5 2023 07:17:04 UTC
18-Jun-2023 07:49:57.894 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Server version number: 9.0.76.0
18-Jun-2023 07:49:57.895 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log OS Name:               Linux
18-Jun-2023 07:49:57.898 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log OS Version:            5.4.0-152-generic
18-Jun-2023 07:49:57.899 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Architecture:          amd64
18-Jun-2023 07:49:57.899 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log Java Home:             /opt/java/openjdk
18-Jun-2023 07:49:57.900 INFO [main] org.apache.catalina.startup.VersionLoggerListener.log JVM Version:           17.0.7+7
...

18-Jun-2023 07:49:58.777 INFO [main] org.apache.coyote.AbstractProtocol.init Initializing ProtocolHandler ["http-nio-8080"]
18-Jun-2023 07:49:58.853 INFO [main] org.apache.catalina.startup.Catalina.load Server initialization in [1340] milliseconds
18-Jun-2023 07:49:58.944 INFO [main] org.apache.catalina.core.StandardService.startInternal Starting service [Catalina]
18-Jun-2023 07:49:58.944 INFO [main] org.apache.catalina.core.StandardEngine.startInternal Starting Servlet engine: [Apache Tomcat/9.0.76]
18-Jun-2023 07:49:58.976 INFO [main] org.apache.coyote.AbstractProtocol.start Starting ProtocolHandler ["http-nio-8080"]
18-Jun-2023 07:49:59.009 INFO [main] org.apache.catalina.startup.Catalina.start Server startup in [155] milliseconds

```

# depends
```
➜  ~ sudo ldd /proc/$PID/root/opt/java/openjdk/bin/java           
	linux-vdso.so.1 (0x00007ffff7fce000)
	libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007ffff7f9e000)
	libjli.so => /proc/38123/root/opt/java/openjdk/bin/../lib/libjli.so (0x00007ffff7f8b000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007ffff7f68000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007ffff7f62000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007ffff7d70000)
	/lib64/ld-linux-x86-64.so.2 (0x00007ffff7fcf000)


Alpine Linux

Golang

```