

#### Turning on GC logging at runtime

> https://plumbr.io/blog/garbage-collection/turning-on-gc-logging-at-runtime

The *-XX:+PrintFlagsFinal* lists all the JVM options, out of which the "*manageable*" options are currently of interest. These are dynamically writeable through the JDK management interface (*com.sun.management.HotSpotDiagnosticMXBean API*). The very same MBean is also published through [JConsole](http://docs.oracle.com/javase/7/docs/technotes/guides/management/figures/memtab.gif). To my liking, the command-line version is a lot more convenient though.


```bash
my-precious me$ jinfo -flag +PrintGCDetails 12278
my-precious me$ jinfo -flag +PrintGC 12278
```

#### With Arthas

[不重启应用怎么查GC日志？Arthas来搞定 ](https://mp.weixin.qq.com/s/GF3C7RcEPV0f1hDah6CJPA)