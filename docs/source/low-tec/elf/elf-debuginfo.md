


## Debug info

### dwarf

Readelf can display and decode the DWARF data in an object or executable file. The
options are
```bash
readelf -w ./envoy
objdump -W ./envoy

objdump --dwarf ./bin
```

> Unwinding a Bug - How C++ Exceptions Work : http://stffrdhrn.github.io/software/toolchain/openrisc/2020/12/13/cxx-exception-unwinding.html 
>
> ELF binaries provide debugging information in a data format called [DWARF](https://en.wikipedia.org/wiki/DWARF). The name was chosen to maintain a fantasy theme. Lately the Linux community has a new debug format called [ORC](https://lwn.net/Articles/728339/).
>
> Though DWARF is a debugging format and usually stored in `.debug_frame`, `.debug_info`, etc sections, a stripped down version it is used for exception handling.
>
> Each ELF binary that supports unwinding contains the `.eh_frame` section to provide unwinding information. This can be seen with the `readelf` program.
>
> We can decode the metadata using `readelf` as well using the `--debug-dump=frames-interp` and `--debug-dump=frames` arguments.
>
> The `frames` dump provides a raw output of the DWARF metadata for each frame. This is not usually as useful as `frames-interp`, but it shows how the DWARF format is actually a bytecode. The DWARF interpreter needs to execute these operations to understand how to derive the values of registers based current PC.
>
> There is an interesting talk in [Exploiting the hard-working DWARF.pdf](https://www.cs.dartmouth.edu/~sergey/battleaxe/hackito_2011_oakley_bratus.pdf).
>
> 



### BACKTRACES/UNWIND TABLES

> https://gnu.wildebeest.org/blog/mjw/2016/02/02/where-are-your-symbols-debuginfo-and-sources/

To make it possible to get accurate and precise backtraces in any context always use gcc `-fasynchronous-unwind-tables` everywhere. It is already the default on the most widely used architectures, but you should enable it on all architectures you support. Fedora already does this (either through making it the default in gcc or by passing it explicitly in the build flags used by rpmbuild).

This will get you unwind tables which are put into `.eh_frame` sections, which are always kept with the binary and loaded in memory and so can be accessed easily and fast. frame pointers only get you so far. It is always the tricky code, signal handlers, fork/exec/clone, unexpected termination in the prologue/epilogue that manipulates the frame pointer. And it is often this tricky situations where you want accurate backtraces the most and you get bad results when only trying to rely on frame pointers. Maintaining frame pointers bloats code and reduces optimization oppertunities. GCC is really good at automatically generating it for any higher level language. And glibc now has `CFI` for all/most hand written assembler.

## FULL DEBUGINFO

> https://gnu.wildebeest.org/blog/mjw/2016/02/02/where-are-your-symbols-debuginfo-and-sources/

Other debug information can be stored separately from the main executable, but we still need to generate it. Some recommendations:

- Always use -g (-gdwarf-4 is the default in recent GCC)
- Do NOT disable -fvar-tracking-assignments
- gdb-add-index (.gdb_index)
- Maybe use -g3 (adds macro definitions)

This is a lot of info and sadly somewhat entangled. But please always generate it and then strip it into a separate .debug file.Not using `-fvar-tracking-assignments`, which is the default with gcc now, really provides very poor results. Some projects disable it because they are afraid that generating extra debuginfo will somehow impact the generated code. If it ever does that is a bug in GCC. If you do want to double check then you can enable GCC [-fcompare-debug](https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html#index-fcompare-debug-1332) or define the environment variable GCC_COMPARE_DEBUG to explicitly make GCC check this invariant isn’t violated.



## How `perf` unwind stack trace
> https://stackoverflow.com/questions/38277463/how-does-linuxs-perf-utility-understand-stack-traces
> http://www.nongnu.org/libunwind/man/libunwind(3).html

