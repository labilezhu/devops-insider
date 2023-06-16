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