

> https://jakubstransky.com/category/java/jvm/

-   **escape analysis** -- the sole purpose of this technique is to decide whether work done in a method is visible outside of the method or has any side effects. This optimisation is performed after any inlining has completed. Such knowledge is utilised in eliminating unnecessary heap allocation via optimisation called *scalar replacement*. In principle, object fields become scalar values as if they had been allocated as local variables instead. This reduces the object allocation rate and reduces memory pressure and in the end results in fewer GC cycles. This effect is nicely demonstrated in ["Automatic stack allocation in the java virtual machine" blog post](https://www.stefankrause.net/wp/?p=64). Details can be also found in [OpenJDK Escape Analysis](https://wiki.openjdk.java.net/display/HotSpot/EscapeAnalysis).
    Escape analysis is also utilised when optimizing a performance of intrinsic locks (those using synchronized). There are possible lock optimisations which and essentially eliminates lock overhead:
    -   *lock elision* -- removing locks on an object which doesn't escape given scope. A great [blog post from Aleksey Shipilёv on lock elision](https://shipilev.net/jvm-anatomy-park/19-lock-elision/) demonstrates the performance effects.
    -   *lock coarsening* -- merges sequential lock regions that share the same lock. More detailed information can be found in the post dedicated to [lock coarsening](http://work.tinou.com/2009/06/lock-coarsening-biased-locking-escape-analysis-for-dummies.html)
    -   *nested locks* -- detects blocks of code where the same lock is acquired without releasing


