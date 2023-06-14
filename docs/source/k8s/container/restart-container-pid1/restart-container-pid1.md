# restart container pid 1


```bash
sudo su

export POD="netshoot-worknode5-0"
ENVOY_PIDS=$(pgrep sleep)
while IFS= read -r ENVOY_PID; do
    HN=$(sudo nsenter -u -t $ENVOY_PID hostname)
    if [[ "$HN" = "$POD" ]]; then # space between = is important
        sudo nsenter -u -t $ENVOY_PID hostname
        export POD_PID=$ENVOY_PID
    fi
done <<< "$ENVOY_PIDS"
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


```

https://dnaeon.github.io/remove-file-handles-with-gdb/
https://support.sas.com/documentation/onlinedoc/ccompiler/doc/lr2/execv.htm

