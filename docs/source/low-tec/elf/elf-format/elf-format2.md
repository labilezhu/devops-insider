## Ref.

> https://greek0.net/elf.html


> An ELF file provides 2 views on the data it contains: A linking view and an execution view. Those two views can be accessed by two headers: the section header table and the program header table.

### Linking view: Section Header Table (SHT)

> The SHT gives an overview on the sections contained in the ELF file. Of particular interest are `REL` sections (relocations), `SYMTAB/DYNSYM` (symbol tables), `VERSYM`/`VERDEF`/`VERNEED` sections (symbol versioning information).

```
readelf -S  ./lib/x86_64-linux-gnu/libc.so.6

There are 73 section headers, starting at offset 0x1eeb10:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .note.gnu.build-i NOTE             0000000000000270  00000270
       0000000000000024  0000000000000000   A       0     0     4
  [ 2] .note.ABI-tag     NOTE             0000000000000294  00000294
       0000000000000020  0000000000000000   A       0     0     4
  [ 3] .gnu.hash         GNU_HASH         00000000000002b8  000002b8
       0000000000003c30  0000000000000000   A       4     0     8
  [ 4] .dynsym           DYNSYM           0000000000003ee8  00003ee8
       000000000000dae8  0000000000000018   A       5     1     8
  [ 5] .dynstr           STRTAB           00000000000119d0  000119d0
       0000000000005ede  0000000000000000   A       0     0     1
       
  [19] .eh_frame_hdr     PROGBITS         00000000001bdd0c  001bdd0c
       00000000000059e4  0000000000000000   A       0     0     4
  [20] .eh_frame         PROGBITS         00000000001c36f0  001c36f0
       000000000001fa70  0000000000000000   A       0     0     8
       
       
  [31] .dynamic          DYNAMIC          00000000003eab80  001eab80
       00000000000001e0  0000000000000010  WA       5     0     8



```



### Execution view: Program Header Table (PHT)

> The PHT contains information for the kernel on how to start the program. The `LOAD` directives determinate what parts of the ELF file get mapped into memory. The `INTERP` directive specifies an ELF interpreter, which is normally `/lib/ld-linux.so.2` on Linux systems.
>
> The `DYNAMIC` entry points to the `.dynamic` section which contains information used by the ELF interpreter to setup the binary.

```
readelf -l  ./lib/x86_64-linux-gnu/libc.so.6

Elf file type is DYN (Shared object file)
Entry point 0x21d10
There are 10 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000000040 0x0000000000000040
                 0x0000000000000230 0x0000000000000230  R      0x8
  INTERP         0x00000000001bdcf0 0x00000000001bdcf0 0x00000000001bdcf0
                 0x000000000000001c 0x000000000000001c  R      0x10
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000001e6a60 0x00000000001e6a60  R E    0x200000
  LOAD           0x00000000001e7618 0x00000000003e7618 0x00000000003e7618
                 0x0000000000005248 0x00000000000094c8  RW     0x200000
  DYNAMIC        0x00000000001eab80 0x00000000003eab80 0x00000000003eab80
                 0x00000000000001e0 0x00000000000001e0  RW     0x8
  NOTE           0x0000000000000270 0x0000000000000270 0x0000000000000270
                 0x0000000000000044 0x0000000000000044  R      0x4
  TLS            0x00000000001e7618 0x00000000003e7618 0x00000000003e7618
                 0x0000000000000010 0x0000000000000090  R      0x8
  GNU_EH_FRAME   0x00000000001bdd0c 0x00000000001bdd0c 0x00000000001bdd0c
                 0x00000000000059e4 0x00000000000059e4  R      0x4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x00000000001e7618 0x00000000003e7618 0x00000000003e7618
                 0x00000000000039e8 0x00000000000039e8  R      0x1

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .interp 
   02     .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_d .gnu.version_r .rela.dyn .rela.plt .plt .plt.got .text __libc_freeres_fn __libc_thread_freeres_fn .rodata .stapsdt.base .interp .eh_frame_hdr .eh_frame .gcc_except_table .hash 
   03     .tdata .init_array __libc_subfreeres __libc_atexit __libc_thread_subfreeres __libc_IO_vtables .data.rel.ro .dynamic .got .got.plt .data .bss 
   04     .dynamic 
   05     .note.gnu.build-id .note.ABI-tag 
   06     .tdata .tbss 
   07     .eh_frame_hdr 
   08     
   09     .tdata .init_array __libc_subfreeres __libc_atexit __libc_thread_subfreeres __libc_IO_vtables .data.rel.ro .dynamic .got 

```



### Exported symbols

```
readelf -D -s  ./lib/x86_64-linux-gnu/libc.so.6

Symbol table for image:
  Num Buc:    Value          Size   Type   Bind Vis      Ndx Name
  490   0: 00000000000bd7e0   532 FUNC    GLOBAL DEFAULT  13 __mbrtowc
  985   0: 0000000000117f60   128 FUNC    GLOBAL DEFAULT  13 __setmntent
 1669   0: 000000000003dc70    60 FUNC    WEAK   DEFAULT  13 isnanl
  809   1: 00000000000300a0   162 FUNC    WEAK   DEFAULT  13 freelocale
   33   1: 0000000000133530   276 FUNC    GLOBAL DEFAULT  13 __vswprintf_chk
  986   1: 000000000011cc50   225 FUNC    WEAK   DEFAULT  13 hcreate_r
 2274   2: 0000000000103df0    22 FUNC    GLOBAL DEFAULT  13 getopt_long_only
 1066   3: 0000000000152550   202 FUNC    GLOBAL DEFAULT  13 endrpcent
  493   3: 0000000000130460    35 FUNC    GLOBAL DEFAULT  13 pthread_mutex_lock

  178 893: 000000000007ef10   279 FUNC    GLOBAL DEFAULT  13 fopen
  132  58: 0000000000043240    26 FUNC    GLOBAL DEFAULT  13 exit
  537 247: 00000000000407e0   572 FUNC    GLOBAL DEFAULT  13 abort
  
  root@worknode6:/proc/3689/root/usr/local/bin# readelf -D -s  /lib/x86_64-linux-gnu/libc.so.6 | grep writev
  520  94: 0000000000123100    40 FUNC    GLOBAL DEFAULT  16 process_vm_writev
  899 250: 0000000000117950   340 FUNC    GLOBAL DEFAULT  16 pwritev64v2
 1958 289: 0000000000117730   177 FUNC    GLOBAL DEFAULT  16 pwritev64
   59 501: 0000000000117950   340 FUNC    GLOBAL DEFAULT  16 pwritev2
 1604 744: 0000000000117730   177 FUNC    GLOBAL DEFAULT  16 pwritev
  898 760: 00000000001175d0   153 FUNC    WEAK   DEFAULT  16 writev
   59  27: 0000000000117950   340 FUNC    GLOBAL DEFAULT  16 pwritev2
  520 232: 0000000000123100    40 FUNC    GLOBAL DEFAULT  16 process_vm_writev
  898 398: 00000000001175d0   153 FUNC    WEAK   DEFAULT  16 writev
  899 399: 0000000000117950   340 FUNC    GLOBAL DEFAULT  16 pwritev64v2
 1604 704: 0000000000117730   177 FUNC    GLOBAL DEFAULT  16 pwritev
 1958 840: 0000000000117730   177 FUNC    GLOBAL DEFAULT  16 pwritev64


```



### Show Require .so

```
root@worknode6:/proc/3689/root/usr/local/bin# readelf -d envoy | grep NEEDED
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [librt.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libdl.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libpthread.so.0]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [ld-linux-x86-64.so.2]

```

```
ldd
```





