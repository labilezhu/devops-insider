

View the .so info:


```
➜  ~ /lib/aarch64-linux-gnu/libc-2.33.so 
GNU C Library (Ubuntu GLIBC 2.33-0ubuntu5) release release version 2.33.
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.
Compiled by GNU CC version 10.2.1 20210320.
libc ABIs: UNIQUE ABSOLUTE
For bug reporting instructions, please see:
<https://bugs.launchpad.net/ubuntu/+source/glibc/+bugs>.
```


View executable deps libs:
```
➜  ~ ldd /usr/bin/sleep
	linux-vdso.so.1 (0x0000ffffb089d000)
	libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000ffffb06bd000)
	/lib/ld-linux-aarch64.so.1 (0x0000ffffb086a000)
```


## Ref
[The Linux Programming Interface]

