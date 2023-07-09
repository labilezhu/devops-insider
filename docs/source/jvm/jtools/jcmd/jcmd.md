
> https://docs.oracle.com/javase/8/docs/technotes/guides/troubleshoot/tooldescr006.html



> http://codefun007.xyz/a/article_detail/764.htm


```bash
jcmd PerfCounter
```

```
性能计数器 重点 信息说明


加载的类数量
java.cls.loadedClasses=4821
卸载的类数量
java.cls.unloadedClasses=0

持久代 或 元数据空间溢出时  可查看 类加载与卸载情况

jvm启动参数  堆大小 GC参数 等
java.rt.vmArgs  

java.rt.vmFlags=""
可通过 jcmd 57383  VM.flags 查看
-XX:CICompilerCount=3 -XX:CMSInitiatingOccupancyFraction=70 -XX:+CMSParallelRemarkEnabled -XX:+FlightRecorder -XX:GCLogFileSize=10485760 -XX:InitialHeapSize=4294967296 -XX:MaxHeapSize=4294967296 -XX:MaxNewSize=348913664 -XX:MaxTenuringThreshold=6 -XX:MinHeapDeltaBytes=196608 -XX:NewSize=348913664 -XX:NumberOfGCLogFiles=3 -XX:OldPLABSize=16 -XX:OldSize=3946053632 -XX:+PrintGC -XX:+PrintGCDetails -XX:+PrintGCTimeStamps -XX:ThreadStackSize=256 -XX:+UnlockCommercialFeatures -XX:+UseCMSCompactAtFullCollection -XX:+UseCMSInitiatingOccupancyOnly -XX:+UseCompressedClassPointers -XX:+UseCompressedOops -XX:+UseConcMarkSweepGC -XX:+UseFastAccessorMethods -XX:+UseFastUnorderedTimeStamps -XX:+UseGCLogFileRotation -XX:+UseParNewGC 



# 线程数  后台线程数 当前活跃线程数  最大活跃线程数 总共启动的线程数 
java.threads.daemon=61
java.threads.live=62
java.threads.livePeak=64
java.threads.started=71

编译线程数
sun.ci.threads=3

使用的GC 策略 算法 
sun.gc.policy.name="ParNew:CMS"

sun.gc.cause="No GC"
sun.gc.lastCause="Allocation Failure"

sun.gc.collector.0.invocations=49
sun.gc.collector.0.lastEntryTime=1063813746003
sun.gc.collector.0.lastExitTime=1063882216126
sun.gc.collector.0.name="PCopy"
sun.gc.collector.0.time=2479494927
sun.gc.collector.1.invocations=2
sun.gc.collector.1.lastEntryTime=13048280096
sun.gc.collector.1.lastExitTime=13080592659
sun.gc.collector.1.name="CMS"
sun.gc.collector.1.time=44870249

sun.gc.generation.0.capacity=348913664
sun.gc.generation.0.maxCapacity=348913664
sun.gc.generation.0.minCapacity=348913664
sun.gc.generation.0.name="new"


sun.gc.generation.0.space.0.capacity=279183360
sun.gc.generation.0.space.0.initCapacity=0
sun.gc.generation.0.space.0.maxCapacity=279183360
sun.gc.generation.0.space.0.name="eden"
sun.gc.generation.0.space.0.used=157804448

sun.gc.generation.0.space.1.capacity=34865152
sun.gc.generation.0.space.1.initCapacity=0
sun.gc.generation.0.space.1.maxCapacity=34865152
sun.gc.generation.0.space.1.name="s0"
sun.gc.generation.0.space.1.used=0

sun.gc.generation.0.space.2.capacity=34865152
sun.gc.generation.0.space.2.initCapacity=0
sun.gc.generation.0.space.2.maxCapacity=34865152
sun.gc.generation.0.space.2.name="s1"
sun.gc.generation.0.space.2.used=34865152

sun.gc.generation.0.spaces=3
sun.gc.generation.0.threads=4

sun.gc.generation.1.capacity=3946053632
sun.gc.generation.1.maxCapacity=3946053632
sun.gc.generation.1.minCapacity=3946053632
sun.gc.generation.1.name="old"

sun.gc.generation.1.space.0.capacity=3946053632
sun.gc.generation.1.space.0.initCapacity=3946053632
sun.gc.generation.1.space.0.maxCapacity=3946053632
sun.gc.generation.1.space.0.name="old"
sun.gc.generation.1.space.0.used=417808824

sun.gc.generation.1.spaces=1

sun.gc.metaspace.capacity=33603584
sun.gc.metaspace.maxCapacity=1105199104
sun.gc.metaspace.minCapacity=0
sun.gc.metaspace.used=32716848


sun.gc.policy.collectors=2
sun.gc.policy.desiredSurvivorSize=17432576
sun.gc.policy.generations=3
sun.gc.policy.maxTenuringThreshold=6
sun.gc.policy.name="ParNew:CMS"
sun.gc.policy.tenuringThreshold=2


sun.gc.tlab.alloc=34922204
sun.gc.tlab.allocThreads=48
sun.gc.tlab.fastWaste=18
sun.gc.tlab.fills=941
sun.gc.tlab.gcWaste=372003
sun.gc.tlab.maxFastWaste=13
sun.gc.tlab.maxFills=53
sun.gc.tlab.maxGcWaste=73779
sun.gc.tlab.maxSlowAlloc=16
sun.gc.tlab.maxSlowWaste=556
sun.gc.tlab.slowAlloc=488
sun.gc.tlab.slowWaste=7472

```



