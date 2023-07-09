

> https://developers.redhat.com/blog/2017/04/04/openjdk-and-containers

Why is it when I specify -Xmx=1g my JVM uses up more memory than 1gb of memory?

Specifying -Xmx=1g is telling the JVM to allocate a 1gb heap. It's not telling the JVM to limit its entire memory usage to 1gb. There are card tables, code caches, and all sorts of other off heap data structures. The parameter you use to specify total memory usage is -XX:MaxRAM. Be aware that with -XX:MaxRam=500m your heap will be approximately 250mb.

