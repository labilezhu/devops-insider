# TCP TIME-WAIT

## Linux 2MSL

> [Coping with the TCP TIME-WAIT state on busy Linux servers](https://vincent.bernat.ch/en/blog/2014-tcp-time-wait-state-linux)
>
> RFC 793 requires the TIME-WAIT state to last twice the time of the MSL. On Linux, this duration is not tunable and is defined in include/net/tcp.h as one minute:
>
> ```c
> #define TCP_TIMEWAIT_LEN (60*HZ) /* how long to wait to destroy TIME-WAIT
>                                   * state, about 60 seconds     */
> ```
>
> 



> [Lower the conntrack tracking time for TIME_WAIT connections](https://gerrit.wikimedia.org/r/c/operations/puppet/+/240361/)
>
> TCP connections in `TIME_WAIT` are maintained for sixty seconds
> by the Linux kernel.
>
> <mark>Note: There's many misleading Google hits indicating that this
> is configurable through the sysctl value `net.ipv4.tcp_fin_timeout`
> but after some digging that turned out to be bogus.</mark>
> It's a constant defined in the Linux lernel in include`/net/tcp.h`:
>
> However `nf_conntrack` tracks these for 120 seconds by default (configurable through the sysctl value `nf_conntrack_tcp_timeout_time_wait`) Reduce this to 65 (the maximum time used by the kernel plus
> a five seconds error margin).



