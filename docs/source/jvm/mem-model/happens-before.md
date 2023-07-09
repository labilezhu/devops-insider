

> https://wiki.sei.cmu.edu/confluence/display/java/Concurrency%2C+Visibility%2C+and+Memory
> https://docs.oracle.com/javase/specs/jls/se8/html/jls-17.html#jls-17.4.5

1.  An unlock on a monitor happens-before every subsequent lock on that monitor.
2.  A write to a volatile field happens-before every subsequent read of that field.
3.  A call to `start()` on a thread happens-before any actions in the started thread.
4.  All actions in a thread happen-before any other thread successfully returns from a `join()` on that thread.
5.  The default initialization of any object happens-before any other actions (other than default-writes) of a program.
6.  A thread calling interrupt on another thread happens-before the interrupted thread detects the interrupt
7.  The end of a constructor for an object happens-before the start of the finalizer for that object

