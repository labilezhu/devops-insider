

The `JAVA_TOOL_OPTIONS` Environment Variable
============================================

> [The JAVA_TOOL_OPTIONS Environment Variable](https://docs.oracle.com/javase/8/docs/technotes/guides/troubleshoot/envvars002.html)


In many environments the command line is not readily accessible to start the application with necessary command-line options. This often arises with applications that use embedded VMs (meaning they use the Java Native Interface (JNI) Invocation API to start the VM), or where the startup is deeply nested in scripts. In these environments the `JAVA_TOOL_OPTIONS` environment variable can be useful to augment a command line.

When this environment variable is set, the `JNI_CreateJavaVM` function (in the JNI Invocation API) prepends the value of the environment variable to the options supplied in its `JavaVMInitArgs` argument.

| **Note:**In some cases this option is disabled for security reasons (for example, on Oracle Solaris operating system this option is disabled when the effective user or group ID differs from the real ID). |
| :-- |

This environment variable allows you to specify the initialization of tools, specifically the launching of native or Java programming language agents using the `-agentlib` or `-javaagent` options. In the following example the environment variable is set so that the HPROF profiler is launched when the application is started:

```
$ export JAVA_TOOL_OPTIONS="-agentlib:hprof"

```

This variable can also be used to augment the command line with other options for diagnostic purposes. For example, you can supply the `-XX:OnError` option to specify a script or command to be executed when a fatal error occurs.

Since this environment variable is examined at the time the `JNI_CreateJavaVM` function is called, it cannot be used to augment the command line with options that would normally be handled by the launcher, for example, VM selection using the `-client` option or the `-server` option.

For more information on `JAVA_TOOL_OPTIONS` environment variable, see Java tool options from [JVM Tool Interface](https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html#tooloptions) documentation.


> https://jarekprzygodzki.dev/post/debugging-any-jvm-in-a-docker-container/

Thanks to [JAVA_TOOL_OPTIONS](https://docs.oracle.com/javase/8/docs/technotes/guides/troubleshoot/envvars002.html) variable it's easy to run any JVM-based Docker image in debug mode. All we have to do is add environment variable definition `JAVA_TOOL_OPTIONS=-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=5005` in docker run or docker-compose.yml and expose port to connect debugger.