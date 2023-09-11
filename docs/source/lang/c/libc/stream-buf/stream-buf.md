# Stream Buffer

```{toctree}
stdout-buffering.md
stdio_buffering.md
```


## tools

### unbuffer
> https://expect.sourceforge.net/example/unbuffer.man.html



**unbuffer** disables the output buffering that occurs when program output is redirected. For example, suppose you are watching the output from a fifo by running it through od and then more.

```
    od -c /tmp/fifo | more
```

You will not see anything until a full page of output has been produced.

You can disable this automatic buffering as follows:



```
    unbuffer od -c /tmp/fifo | more
```

When you have a pipeline, unbuffer must be applied to each element except the last (since that doesn't have its output redirected). Example:

```
        unbuffer p1 | unbuffer p2 | unbuffer p3 | p4
```



