

> https://serverfault.com/questions/48330/how-can-i-get-more-info-on-open-pipes-show-in-proc-in-linux

In Linux if you go digging in /proc/<pid>/fd often you'll see output like:

```
lrwx------ 1 root root 64 Jul 30 15:14 0 -> /dev/null
lrwx------ 1 root root 64 Jul 30 15:14 1 -> /dev/null
l-wx------ 1 root root 64 Jul 30 15:14 10 -> pipe:[90222668]
lr-x------ 1 root root 64 Jul 30 15:14 11 -> pipe:[90222669]
l-wx------ 1 root root 64 Jul 30 15:14 13 -> pipe:[90225058]
lr-x------ 1 root root 64 Jul 30 15:14 14 -> pipe:[90225059]
```


```
lsof -E | grep 119940
```


```bash
export PIPE_ID=119941


(find /proc -type l | xargs ls -l | fgrep "pipe:[$PIPE_ID]") 2>/dev/null

```