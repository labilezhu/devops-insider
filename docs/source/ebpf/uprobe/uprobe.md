
### List uprobe

```
bpftrace -l 'uprobe:/bin/bash:*'
```

> https://www.kernel.org/doc/html/latest/trace/uprobetracer.html


```bash
# Print out the events that are registered:

cat /sys/kernel/debug/tracing/uprobe_events
```