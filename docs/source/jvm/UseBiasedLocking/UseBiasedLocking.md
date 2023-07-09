> https://jet-start.sh/blog/2020/06/23/jdk-gc-benchmarks-rematch

`-XX:-UseBiasedLocking`: biased locking has for a while been under criticism that it causes higher latency spikes due to bias revocation that must be done within a GC safepoint. In the upcoming JDK version 15, biased locking will be [disabled by default and deprecated](https://openjdk.java.net/jeps/374). Any low-latency Java application should have this disabled and we disabled it in all our measurements.

