
### Can we get compiler information from an elf binary?

There isn't a universal way, but you can make an educated guess by looking for things only done by one compiler.

GCC is the easiest; it writes a .comment section that contains the GCC version string (the same string you get if you run gcc --version). I don't know if there's a way to display it with readelf, but with objdump it's:

```
objdump -s --section .comment /path/binary

readelf -p .comment ./envoy
```


## BUILD-ID

> https://gnu.wildebeest.org/blog/mjw/2016/02/02/where-are-your-symbols-debuginfo-and-sources/

```log
$ readelf -n /bin/bash
...
Displaying notes found at file offset 0x00000274 with length 0x00000024:
  Owner                 Data size   Description
  GNU                  0x00000014   NT_GNU_BUILD_ID (unique build ID bitstring)
    Build ID: 54967822da027467f21e65a1eac7576dec7dd821
```