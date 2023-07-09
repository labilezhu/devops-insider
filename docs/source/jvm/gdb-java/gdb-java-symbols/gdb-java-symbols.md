

> https://github.com/jvm-profiling-tools/async-profiler/wiki/Installing-Debug-Symbols


### Installing Debug Symbols

The allocation profiler requires HotSpot debug symbols. **Oracle JDK already has them embedded in libjvm.so**, but in OpenJDK builds they are typically shipped in a separate package. For example, to install OpenJDK debug symbols on Debian / Ubuntu, run:

```bash
apt install openjdk-8-dbg
```

or for OpenJDK 11:

```bash
apt install openjdk-11-dbg
```


The gdb tool can be used to verify if the debug symbols are properly installed for the libjvm library. For example on Linux:

```bash
$ gdb $JAVA_HOME/lib/server/libjvm.so -ex 'info address UseG1GC'
```
This command's output will either contain Symbol "UseG1GC" is at 0xxxxx or No symbol "UseG1GC" in current context.