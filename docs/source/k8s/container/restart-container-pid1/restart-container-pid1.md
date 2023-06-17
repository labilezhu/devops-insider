# exec hot swap


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

```bash
ssh labile@192.168.1.55

export POD="netshoot-worknode5-0"
PIDS=$(pgrep nc)
while IFS= read -r TPID; do
    HN=$(sudo nsenter -u -t $TPID hostname)
    if [[ "$HN" = "$POD" ]]; then # space between = is important
        sudo nsenter -u -t $TPID hostname
        export POD_PID=$TPID
    fi
done <<< "$PIDS"
echo $POD_PID
export PID=$POD_PID

➜  ps -f $PID
UID          PID    PPID  C STIME TTY      STAT   TIME CMD
root       16018   15931  0 02:31 ?        Ss     0:00 /usr/bin/nc -k -l 8080

➜  sudo ls -l /proc/$PID/fd
total 0
lrwx------ 1 root root 64 Jun 17 02:31 0 -> /dev/null
l-wx------ 1 root root 64 Jun 17 02:31 1 -> 'pipe:[85916]'
l-wx------ 1 root root 64 Jun 17 02:31 2 -> 'pipe:[85917]'
lrwx------ 1 root root 64 Jun 17 02:31 3 -> 'socket:[86567]'

➜  sudo nsenter -n -t $PID ss -tpna | grep $PID
LISTEN   0        1                0.0.0.0:8080          0.0.0.0:*      users:(("nc",pid=16018,fd=3))


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