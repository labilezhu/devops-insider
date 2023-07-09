

## When Are Explicit JMX Connections Necessary?

-   The target application is running on the same host as Java VisualVM but was started by a different user than the one who started Java VisualVM. Java VisualVM discovers running applications using the `jps` tool ([Solaris, Linux, or Mac OS X](https://docs.oracle.com/javase/8/docs/technotes/tools/unix/jps.html)), which can only discover Java applications started by  the same user as the one who starts the Java VisualVM tool.


-   The target application is running on a remote host where `jstatd` ([Solaris, Linux, or Mac OS X](https://docs.oracle.com/javase/8/docs/technotes/tools/unix/jstatd.html) or [Windows](https://docs.oracle.com/javase/8/docs/technotes/tools/windows/jstatd.html)) is not running or is running but was started by a different user. The `jstatd` daemon provides an interface that allows remote monitoring applications to connect to Java applications on the host where it is running.

