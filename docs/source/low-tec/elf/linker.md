> Ref. [Learning Linux Binary Analys]

### The LD_PRELOAD environment variable
The `LD_PRELOAD` environment variable can be set to specify a library path that should
be dynamically linked before any other libraries. This has the effect of allowing
functions and symbols from the preloaded library to override the ones from the other
libraries that are linked afterwards. This essentially allows you to perform runtime
patching by redirecting shared library functions. As we will see in later chapters, this
technique can be used to bypass anti-debugging code and for userland rootkits.


