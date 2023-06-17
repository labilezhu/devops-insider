> [Hands-on System Proframming with Linux]

### Sleeping correctly

As a simple example, let's say that the process must work this way (pseudo code
follows):
```c
<...>
func_a();
sleep(10);
func_b();
<...>
```
It's quite clear: the process must sleep for 10 seconds; the code shown should work. Is there a problem? **Well, yes, signals: what if the process enters the sleep, but three seconds into the sleep a signal arrives? The default behavior (meaning, unless signals are masked) is to handle the signal, and you would imagine, go back to sleep for the remaining time (seven seconds). But, no, that's not what happens: the sleep is aborted!**

Further, it's important to realize that the sleep(3) API documents that its return
value is the amount of time remaining to sleep; so unless sleep(3) returns 0 , the
sleep is not done! The developer is actually expected to invoke sleep(3) in a loop,
until the return value is 0 .


So, we conclude that just using a sleep(3) in the code is not that great an idea
because of the following:
- The sleep, once interrupted by signal delivery, must be manually restarted.
- The granularity of sleep(3) is very coarse: a second. (A second is a very,
very long time for a modern microprocessor! Many real-world applications
rely on at least millisecond-to-microsecond-level granularity.)


So, what is the solution?


#### The nanosleep system call

Linux provides a system call, nanosleep(2) , that in theory can provide nanosecond-
level granularity, that is, a sleep of a single nanosecond. (Well, in practice, the
granularity will also depend on the resolution of the hardware timer chip on the
board.) This is the prototype of this API:
```c
#include <time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);
```
The system call has two parameters both are pointers to structure of data type struct
timespec ; this structure definition is as follows:
```c
struct timespec {
    time_t tv_sec; /* seconds */
    long tv_nsec; /* nanoseconds */
};
```
Obviously, this allows you to specify the sleep time in seconds and nanoseconds; the
first parameter req is the required time ( s.ns ), the second parameter rem is
the remaining time to sleep. See, the OS helps us out here: if the sleep is interrupted
by a signal (any signal that is non-fatal), the nanosleep system call fails
returning -1 , and errno is set to the value EINTR (Interrupted system call). Not only
that, the OS calculates and returns (into this second pointer, a value-result type of
parameter), the amount of time remaining to sleep accurate to the nanosecond. This
way, we detect the case, set req to rem , and manually reissue the nanosleep(2) to
have the sleep continue until it's fully done.

