# JEP 271: Unified GC Logging

> https://openjdk.org/jeps/271

## Summary

Reimplement GC logging using the unified JVM logging framework introduced in [JEP 158](http://openjdk.java.net/jeps/158).

## Non-Goals

It is not a goal to ensure that current GC log parsers work without change on the new GC logs.

Not all log entries will necessarily be reproduced in the new logging format.

## Description

Reimplement GC logging in a manner that is as consistent as is reasonable with the current GC logging format. There will, of necessity, be some difference between the new and the old formats.

### The "gc" Tag

<mark>The idea is that `-Xlog:gc` (only log on the "gc" tag at info level) should be similar to what `-XX:+PrintGC` did, which was printing one line per GC.</mark> This means that `log_info(gc)("message")` should be used very carefully. Don't log at the info level on just the "gc" tag unless it is the one message that should be printed per GC.

It is fine to log at the info level using the "gc" tag if it is combined with other tags. For example:

```
log_info(gc, heap, ergo)("Heap expanded");
```

The idea here is that `-Xlog:gc` should be somewhat similar to what you used to get with `-XX:+PrintGCDetails`. But this mapping is not as strict as the mapping from `-Xlog:gc` to `-XX:+PrintGC`. The rule for `-XX:+PrintGC` was pretty clear: one line per GC. The rule for `-XX:+PrintGCDetails` has never been very clear. So, some `-XX:+PrintGCDetials` logging may be mapped to several tags and some may just be mapped to the debug level for the "gc" tag.

All GC-related logging should use the "gc" tag. Most logging should not use just the "gc" tag but, rather, combine it with other tags as appropriate.

There are also border-line cases where it is unclear whether "gc" is the appropriate tag, for example in allocation code. Most of these cases should probably not use the "gc" tag.

### Other Tags

There are many other tags besides "gc". Some of them map pretty cleanly to old flags. `PrintAdaptiveSizePolicy`, for example, is more or less mapped to the "ergo" tag (in combination with the "gc" tag and potentially other tags).

### Verbose

Most logging that was guarded by the `Verbose` flag (a develop flag) should be mapped to the trace level. The exception is if it is very expensive logging from a performance perspective, in which case it is mapped to the develop level.

### Prefix

The prefix support in the unified logging framework is used to add the GC id to GC log messages. The GC id is only interesting for logging that happens during a GC. Since the prefixes are defined for a particular tag set, i.e., a combination of tags, it is necessary to make sure that logging that occurs between GCs do not use the same tag set as logging that is done during GCs.

### Dynamic Configuration

Some logging requires that data has been collected at an earlier state. The unified logging framework allows for all logging to be dynamically turned on and off using `jcmd`. This means that for logging that relies on previously-collected data it is not enough to check whether logging is enabled; there must also be checks in place to check that the data is available.


> [JDK-8059805: JEP 271: Unified GC Logging](https://bugs.openjdk.org/browse/JDK-8059805)