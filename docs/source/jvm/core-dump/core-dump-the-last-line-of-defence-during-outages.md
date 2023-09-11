# Core dump - the last line of defence during outages

> From https://krzysztofslusarski.github.io/2021/11/14/coredump.html

#  Core dump - the last line of defence during outages

## Context

There are situations where an application running on the JVM doesn’t respond. It can be so unresponsive that you cannot attach to it with any of the external tools, like **JMX client**, **jcmd**, **jstack**, **jmap** and so on. The first thought may be to restart the JVM. It will probably help, but during such a situation the application is in a specific **state**. If you restart the application, that state is lost.

## The JVM is just a process

You need to understand that the JVM is just a process from an operating system perspective. The OS offers us tools to diagnose unresponsive processes. The easiest way is to fetch a [core dump](https://en.wikipedia.org/wiki/Core_dump). You can do it with a single line:

```
gcore -o <filename> <pid>
```

It will create a very large file `<filename>.<pid>`. This operation may take a while.

## What can we do with such a dump?

Since JDK 9 there is a tool called `jhsdb` delivered in every JDK. With that tool you can manipulate the core dump in multiple ways. The two basic modes are:

### Heap dump

You can fetch the heap dump from a core dump using:

```
jhsdb jmap --binaryheap --dumpfile <output hprof file> --core <core dump file> --exe <path to java exe>
```

**Mind that** the `jhsdb` and `java`, that is pointed as a last argument, must be from exactly the same distribution as the JVM, from which the core dump has been created.

### Thread dump

You can fetch the heap dump from a core dump using:

```
jhsdb jstack --core <core dump file> --exe <path to java exe>
```

## There is more…

There are other modes of the `jhsdb`, see the help of that tool to see if they are useful to you.

You can also find it useful that the core dump is just a process memory snapshot, so you can do everything you could do with other core dumps. For example: you can attach the `gdb` to your dump:

```
gdb <path to java exe> <core dump file>
```

That will open you the GNU debugger. Sometimes this is useful, when you need to check where your threads have hung. Unfortunately there is an assembly code and native stacks, but it is better than nothing.
