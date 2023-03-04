---
date: 2022-04-18T16:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
description: ""
tags:
---



# How to extract and disassemble a Linux kernel image (vmlinuz)

> https://blog.packagecloud.io/how-to-extract-and-disassmble-a-linux-kernel-image-vmlinuz/



## TL;DR

This blog post explains how to extract and disassemble a Linux kernel image. It will cover the `extract-vmlinux` script, how to use `objdump`, and how to use `/boot/System.map` to locate functions and other symbols.

## Extracting the Linux kernel image (vmlinuz)

First, you’ll need to get the `extract-vmlinux` script so that you can decompress and extract the Linux kernel image.

You can download the latest version from [GitHub](https://raw.githubusercontent.com/torvalds/linux/master/scripts/extract-vmlinux):

```
$ wget -O extract-vmlinux https://raw.githubusercontent.com/torvalds/linux/master/scripts/extract-vmlinux
```

It’s unlikely that the script will change, but to be safe you should use the `extract-vmlinux` script that is from the same source tree as your kernel.

If you are extracting a kernel installed from your operating system, you can install the `extract-linux` script with your package manager.

On Ubuntu, install `linux-headers-$(uname -r)`:

```
$ sudo apt-get install linux-headers-$(uname -r)
```

You will be able to find the `extract-linux` script at `/usr/src/linux-headers-$(uname -r)/scripts/extract-vmlinux`.

On CentOS, install `kernel-devel`:

```
$ sudo yum install kernel-devel
```

You will be able to find the `extract-linux` script at `/usr/src/kernels/$(uname -r)/scripts/extract-vmlinux`.

 

## Using `extract-vmlinux`

You can now use `extract-vmlinux` to decompress and extract the kernel image.

A good first step is to create a temporary directory and copy the kernel image to it:

```
$ mkdir /tmp/kernel-extract
$ sudo cp /boot/vmlinuz-$(uname -r) /tmp/kernel-extract/
```

Now, run the `extract-vmlinux` script to extract the image.

On Ubuntu:

```
$ cd /tmp/kernel-extract/
$ sudo /usr/src/linux-headers-$(uname -r)/scripts/extract-vmlinux vmlinuz-$(uname -r) > vmlinux
```

On CentOS:

```
$ cd /tmp/kernel-extract/
$ sudo /usr/src/kernels/$(uname -r)/scripts/extract-vmlinux vmlinuz-$(uname -r) > vmlinux
```

 

## Disassmble the Linux kernel with `objdump`

Now that you have decompressed and extracted the kernel image, you can use `objdump` to disassemble it. There’s quite a bit of code, so piping the output to `less` is probably a good idea.

Using the same directory structure as before:

```
$ cd /tmp/kernel-extract/
$ objdump -D vmlinux | less
```

 

## Finding symbols in `/boot/System.map`

So, you’ve extracted the kernel and are now looking at the disassembled kernel. You’ll notice that there are no symbol names, so you can’t easily find the starting point for functions you want to examine.

Luckily, all the symbols and their starting address can be found in the file `/boot/System.map-$(uname -r)`.

For example, let’s lookup the address of `tcp_v4_do_rcv`:

```
$ sudo grep " tcp_v4_do_rcv"  /boot/System.map-3.2.0-29-virtual
ffffffff81590df0 T tcp_v4_do_rcv
```

You can now search the `objdump` output for the address `ffffffff81590df0` to find the disassmbled `net_ipv4_path` function:

```
ffffffff81590df0:       55                      push   %rbp
ffffffff81590df1:       48 89 e5                mov    %rsp,%rbp
ffffffff81590df4:       48 83 ec 20             sub    $0x20,%rsp
ffffffff81590df8:       48 89 5d e8             mov    %rbx,-0x18(%rbp)
ffffffff81590dfc:       4c 89 65 f0             mov    %r12,-0x10(%rbp)
ffffffff81590e00:       4c 89 6d f8             mov    %r13,-0x8(%rbp)
ffffffff81590e04:       e8 77 9c 0c 00          callq  0xffffffff8165aa80
...
```





## Conclusion

Extracting the Linux kernel is relatively straightforward once you know what `extract-vmlinux` is and where to find it. Extracting the kernel can be useful when you want to verify comments left by kernel code authors or are just curious to see how a particular function was compiled.



# vmlinux-disassembly-symbolizer

> https://githubhot.com/index.php/repo/loskutov/vmlinux-disassembly-symbolizer

Most Linux distributions ship a stripped kernel, with symbol information available separately in a System.map file (e.g. /boot/System.map-$(uname -r)). Attempting to directly disassemble vmlinux on such systems will produce a raw disassembly listing, having no symbol names at all, as tools like objdump are unaware of System.map. This script allows to make the disassembly mostly as beautiful as if the kernel was not stripped, extracting the symbol table from the supplied System.map file and annotating the disassembly correspondingly.

```bash
curl -O -L https://github.com/loskutov/vmlinux-disassembly-symbolizer/raw/master/symbolize_vmlinux_disassembly.py

python3 ./symbolize_vmlinux_disassembly.py /boot/System.map-$(uname -r) <(objdump -D /tmp/kernel-extract/vmlinux) > disasm_sym.asm
```



# Knowledge



## What Is The System.map File?

> There are 2 files that are used as a kernel symbol table:
>
> 1. `/proc/kallsyms`
> 2. `System.map`
>
> There. You now know what the `System.map` file is.
>
> Every time you compile a new kernel, the addresses of various symbol names are bound to change.
>
> `/proc/kallsyms` is a "proc file" that is created on the fly when a kernel boots up. Actually, it's not really a disk file; it's a representation of kernel data which is given the illusion of being a disk file. If you don't believe me, try finding the filesize of `/proc/kallsyms`. Therefore, it will always be correct for the kernel that is currently running.
>
> However, System.map is an actual file on your filesystem. When you compile a new kernel, your old `System.map` has wrong symbol information. A new `System.map` is generated with each kernel compile and you need to replace the old copy with your new copy.



```
sudo grep ip_finish_output2 /boot/System.map-$(uname -r)
ffffffff8196a100 t ip_finish_output2 -> https://elixir.bootlin.com/linux/v5.4/source/net/ipv4/ip_output.c#L185

__local_bh_enable_ip
```

