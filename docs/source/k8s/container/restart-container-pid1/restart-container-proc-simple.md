# exec hot swap

## gdb exec simple way

### setup target e.g pod

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



```bash
sudo gdb -p $PID

➜  ~ ps -f $PID
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       16018   15931  0 02:31 ?        ts     0:00 /usr/bin/nc -k -l 8080

call close(3)
```



# Misc

## restart container pid 1


```bash
sudo su

export POD="netshoot-worknode5-0"
PIDS=$(pgrep fortio)
while IFS= read -r TPID; do
    HN=$(sudo nsenter -u -t $TPID hostname)
    if [[ "$HN" = "$POD" ]]; then # space between = is important
        sudo nsenter -u -t $TPID hostname
        export POD_PID=$TPID
    fi
done <<< "$PIDS"
echo $POD_PID
export PID=$POD_PID

# sudo nsenter -t $PID -u -p -m bash #NO -n
watch "ps -ef --forest | grep  -C5 $PID"

export PPPID=`ps -o ppid= -p $PID|xargs`
export shim_PID=$PPPID
echo $shim_PID

top -p $shim_PID

sudo gdb -p $shim_PID

c&

#################

handle all nostop pass 

handle SIGCHLD print pass

handle SIGCHLD print nopass

# signal 0

sudo kill -9 $PID


```

https://sourceware.org/gdb/onlinedocs/gdb/Signals.html
https://sourceware.org/gdb/onlinedocs/gdb/Signaling.html#Signaling

## exec

```gdb
call execl("/usr/bin/ping","ping",0)

call execl("/usr/bin/cat","cat",0)

p close(3)
call execl("/bin/sleep","sleep","10d",0)


```

https://dnaeon.github.io/remove-file-handles-with-gdb/
https://support.sas.com/documentation/onlinedoc/ccompiler/doc/lr2/execv.htm

https://incoherency.co.uk/blog/stories/closing-a-socket.html

## Redirect fd
> https://www.redpill-linpro.com/sysadvent/2015/12/04/changing-a-process-file-descriptor-with-gdb.html

```bash
$ fdswap /var/log/mydaemon/output.log /dev/null 1234
```


```bash
#!/bin/bash
#
# fdswap
#
if [ "$2" = "" ]; then 
	echo "
    Usage: $0 /path/to/oldfile /path/to/newfile [pids]
    Example: $0 /var/log/daemon.log /var/log/newvolume/daemon.log 1234
    Example: $0 /dev/pts/53 /dev/null 2345"; exit 0
fi

if gdb --version > /dev/null 2>&1; then true
else echo "Unable to find gdb."; exit 1
fi

src="$1"; dst="$2"; shift; shift 
pids=$* 

for pid in ${pids:=$( /sbin/fuser $src | cut -d ':' -f 2 )}; 
do
    echo "src=$src, dst=$dst"
    echo "$src has $pid using it" 
    ( 
        echo "attach $pid" 
        echo 'call open("'$dst'", 66, 0666)'
        for ufd in $(LANG=C ls -l /proc/$pid/fd | \
        grep "$src"\$ | awk ' { print $9; } '); 
 	    do echo 'call dup2($1,'"$ufd"')'; done 
        echo 'call close($1)'
        echo 'detach'; echo 'quit' 
        sleep 5
    ) | gdb -q -x -
done
```

```bash
sudo --preserve-env su --preserve-environment
for ufd in $(LANG=C ls -l /proc/$PID/fd | awk ' { print $9; } ' | egrep -v '^[012]$' ); 
do echo 'call close('"$ufd"')'; done 


call close(10)
call close(11)
call close(3)
call close(4)
call close(5)
call close(6)
call close(7)
call close(8)
call close(9)
```


> [GDB for loop to close leaky fds](https://gist.github.com/labeneator/5049227)


```
(gdb) set $max=8
(gdb) set $current=3
(gdb) while ($current < $max)
 > p close($current++)                                                                                                                                                     
 >end
```


#### Script to inject an exit(0) syscall into a running proces
> https://gist.github.com/moyix/95ca9a7a26a639b2322c36c7411dc3be

```bash
#!/bin/bash

gdb -p "$1" -batch -ex 'set {short}$rip = 0x050f' -ex 'set $rax=231' -ex 'set $rdi=0' -ex 'cont'
```


#### How can I make a specific process exec a given executable with ptrace()
> https://unix.stackexchange.com/questions/464539/how-can-i-make-a-specific-process-exec-a-given-executable-with-ptrace


```c
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#define CONFIG_ARM_THUMB

#ifdef CONFIG_ARM_THUMB
#define thumb_mode(regs) \
        (((regs)->ARM_cpsr & PSR_T_BIT))
#else
#define thumb_mode(regs) (0)
#endif

extern char **environ;

static pid_t pid;

/* The length of a string, plus the null terminator, rounded up to the nearest sizeof(long). */
size_t str_size(char *str) {
        size_t len = strlen(str);
        return len + sizeof(long) - len % sizeof(long);
}

void must_poke(long addr, long data) {
        if(ptrace(PTRACE_POKEDATA, pid, (void*)addr, (void*)data)) {
                perror("ptrace(PTRACE_POKEDATA, ...)");
                exit(1);
        }
}

void must_poke_multi(long addr, long* data, size_t len) {
        size_t i;
        for(i = 0; i < len; ++i) {
                must_poke(addr + i * sizeof(long), data[i]);
        }
}

long must_poke_string(long addr, char* str) {
        size_t len = str_size(str);
        size_t longs_len = len / sizeof(long);
        char *more_nulls_str = malloc(len);
        memset(more_nulls_str + len - sizeof(long), 0, sizeof(long)); /* initialize the bit we might not copy over */
        strcpy(more_nulls_str, str);
        must_poke_multi(addr, (long*)more_nulls_str, longs_len);
        free(more_nulls_str);
        return addr + len;
}

int main(int argc, char** argv) {
        struct user_regs regs;
        int i, envc;
        unsigned long mmap_base;
        size_t mmap_string_offset, mmap_argv_offset, mmap_envp_offset;
        size_t mmap_len = 2 * sizeof(char*); /* for the NULLs at the end of argv and envp */

        if(argc < 3) {
                fprintf(stderr, "Usage: %s <pid> <executable image> <args...>\n", argv[0]);
                return 1;
        }

        pid = strtol(argv[1], NULL, 10);

        /* for the image name */
        mmap_len += str_size(argv[2]);

        for(i = 3; i < argc; ++i) {
                /* for the pointer in argv plus the string itself */
                mmap_len += sizeof(char*) + str_size(argv[i]);
        }

        for(i = 0; environ[i]; ++i) {
                /* for the pointer in envp plus the string itself */
                mmap_len += sizeof(char*) + str_size(environ[i]);
        }
        envc = i;

        if(ptrace(PTRACE_ATTACH, pid, 0, 0)) {
                perror("ptrace(PTRACE_ATTACH, ...)");
                return 1;
        }
        if(waitid(P_PID, pid, NULL, WSTOPPED)) {
                perror("waitid");
                return 1;
        }

        /* Stop at whatever syscall happens to be next */
        if(ptrace(PTRACE_SYSCALL, pid, 0, 0)) {
                perror("ptrace(PTRACE_SYSCALL, ...)");
                return 1;
        }
        printf("Waiting for the target process to make a syscall...\n");
        if(waitid(P_PID, pid, NULL, WSTOPPED)) {
                perror("waitid");
                return 1;
        }
        printf("Target made a syscall. Proceeding with injection.\n");

        if(ptrace(PTRACE_GETREGS, pid, 0, &regs)) {
                perror("ptrace(PTRACE_GETREGS, ...)");
                return 1;
        }

        /* End up back on the syscall instruction so we can use it again */
        regs.ARM_pc -= (thumb_mode(&regs) ? 2 : 4);

        /* mmap some space for the exec parameters */
        regs.ARM_r0 = (long)0;
        regs.ARM_r1 = (long)mmap_len;
        regs.ARM_r2 = (long)(PROT_READ|PROT_WRITE);
        regs.ARM_r3 = (long)(MAP_PRIVATE|MAP_ANONYMOUS);
        regs.ARM_r4 = (long)-1;
        regs.ARM_r5 = (long)0;
        if(ptrace(PTRACE_SETREGS, pid, 0, &regs)) {
                perror("ptrace(PTRACE_SETREGS, ...)");
                return 1;
        }
        if(ptrace(PTRACE_SET_SYSCALL, pid, 0, SYS_mmap2)) {
                perror("ptrace(PTRACE_SET_SYSCALL, ...)");
                return 1;
        }

        /* jump to the end of the syscall */
        if(ptrace(PTRACE_SYSCALL, pid, 0, 0)) {
                perror("ptrace(PTRACE_SYSCALL, ...)");
                return 1;
        }
        if(waitid(P_PID, pid, NULL, WSTOPPED)) {
                perror("waitid");
                return 1;
        }

        /* make sure it worked and get the memory address */
        if(ptrace(PTRACE_GETREGS, pid, 0, &regs)) {
                perror("ptrace(PTRACE_GETREGS, ...)");
                return 1;
        }

        if(regs.ARM_r0 > -4096UL) {
                errno = -regs.ARM_r0;
                perror("traced process: mmap");
                return 1;
        }
        mmap_base = regs.ARM_r0;

        /* set up the execve args in memory */
        mmap_argv_offset = must_poke_string(mmap_base, argv[2]);

        mmap_string_offset = mmap_argv_offset + (argc - 2) * sizeof(char*); /* don't forget the null pointer */
        for(i = 0; i < argc - 3; ++i) {
                must_poke(mmap_argv_offset + i * sizeof(char*), mmap_string_offset);
                mmap_string_offset = must_poke_string(mmap_string_offset, argv[i + 3]);
        }
        must_poke(mmap_argv_offset + (argc - 3) * sizeof(char*), 0);
        mmap_envp_offset = mmap_string_offset;
        mmap_string_offset = mmap_envp_offset + (envc + 1) * sizeof(char*); /* don't forget the null pointer */
        for(i = 0; i < envc; ++i) {
                must_poke(mmap_envp_offset + i * sizeof(char*), mmap_string_offset);
                mmap_string_offset = must_poke_string(mmap_string_offset, environ[i]);
        }
        must_poke(mmap_envp_offset + envc * sizeof(char*), 0);

        /* jump to the start of the next syscall (same PC since we reset it) */
        if(ptrace(PTRACE_SYSCALL, pid, 0, 0)) {
                perror("ptrace(PTRACE_SYSCALL, ...)");
                return 1;
        }
        if(waitid(P_PID, pid, NULL, WSTOPPED)) {
                perror("waitid");
                return 1;
        }

        if(ptrace(PTRACE_GETREGS, pid, 0, &regs)) {
                perror("ptrace(PTRACE_GETREGS, ...)");
                return 1;
        }

        /* call execve */
        regs.ARM_r0 = (long)mmap_base;
        regs.ARM_r1 = (long)mmap_argv_offset;
        regs.ARM_r2 = (long)mmap_envp_offset;
        if(ptrace(PTRACE_SETREGS, pid, 0, &regs)) {
                perror("ptrace(PTRACE_SETREGS, ...)");
                return 1;
        }
        if(ptrace(PTRACE_SET_SYSCALL, pid, 0, SYS_execve)) {
                perror("ptrace(PTRACE_SET_SYSCALL, ...)");
                return 1;
        }

        /* and done. */
        if(ptrace(PTRACE_DETACH, pid, 0, 0)) {
                perror("ptrace(PTRACE_DETACH, ...)");
                return 1;
        }
        return 0;
}
```

# StatefulSet

```yaml
kubectl delete StatefulSet netshoot-worknode5

kubectl apply -f - <<"EOF"

apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: netshoot-worknode5
  labels:
    app: netshoot-worknode5
spec:
  replicas: 1
  selector:
    matchLabels:
      app: netshoot-worknode5
  template:
    metadata:
      labels:
          app.kubernetes.io/name: netshoot-worknode5
          app: netshoot-worknode5
      annotations:
        sidecar.istio.io/inject: "false"
    spec:
      restartPolicy: Always
      imagePullSecrets:
      - name: docker-registry-key
      containers:
      - name: main-app
        command:
        - /usr/bin/nc
        image: docker.io/nicolaka/netshoot:latest
        imagePullPolicy: IfNotPresent
        args: ["-k","-l","8080"]
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