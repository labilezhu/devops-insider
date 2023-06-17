
> [Hands-on System Proframming with Linux]

#### 可重入要求
重入安全函数，一般叫 `reentrant-safe` ，在信号量处理语境下，叫 `async-signal-safe` .

> `async-signal-safe` : invoking the function from within a signal handler
while a previous invocation is still running is safe.

- 安全的堆栈内存
由于信号量处理程序的重入、中断、等特性，在信号量处理程序中访问非堆栈内存时，需要考虑竞态问题。



> As a general rule, functions that use only local variables are reentrant-safe; any usage of a global or a static data renders them unsafe. This is a key point: **you can only call those functions in a signal handler that are documented as being reentrant-safe or signal-async-safe**.
> The man page on signal-safety(7) http://man7.org/​linux/man-​pages/man7/signal-​safety.​7.html provides details for this.


So the bottom line is this: from within a signal handler, you can only invoke the
following:
- C library functions or system calls that are in the signal-safety(7) man page (do look it up)
- Within a third-party library, functions explicitly documented as being async-signal-safe
- Your own library or other functions that have been explicitly written to be async-signal-safe

#### Alternate ways to be safe within a signal handler

What if we must access some global state within our signal handler routine? There do
exist some alternate ways of making it signal-safe:
- At the point you must access these variables, ensure that all signals are blocked (or masked), and, once done, restore the signal state (unmask).
- Perform some kind of locking on shared data while accessing it.
  * In multiprocess applications (the case we are talking about here), (binary) semaphores can be used as a locking mechanism to protect shared data across processes.
  * In multithreaded applications, the use of an appropriate locking mechanism (mutex locks, perhaps; we shall, of course, cover this in a later chapter in detail).

- If your requirement is to just operate upon global integers (a common case for signal handling!), use a special data type (the sig_atomic_t ). Seen later on.

