

## Tech Stack

* bpftrace
* BCC
* LLVM

> The LLVM compiler supports BPF as a compilation target. BPF programs can be written using
> a higher-level language that LLVM supports, such as C (via Clang) or LLVM Intermediate
> Representation (IR), and then compiled into BPF. LLVM includes an optimizer, which improves
> the efficiency and size of the BPF instructions it emits.




## Traditional BPF toolkit

### bpftrace depends

```bash
$ sudo apt install linux-headers-$(uname -r)

$ sudo apt install bpftrace

The following NEW packages will be installed:
  bpftrace libbpfcc libclang1-9 libllvm9


# shared library for BPF Compiler Collection (BCC)
apt show libbpfcc
dpkg -L libbpfcc

# LLVM runtime library.
apt show libllvm9
dpkg -L libllvm9

# C interface to the Clang library
apt show libclang1-9
dpkg -L libclang1-9

```

### BCC depends

```bash
$ sudo apt-get install bpfcc-tools 

The following additional packages will be installed:
  ieee-data python3-bpfcc python3-netaddr
Suggested packages:
  ipython3 python-netaddr-docs
```



## Clang

```
$ sudo apt install clang

The following additional packages will be installed:
  binfmt-support clang-10 gcc-9-base lib32gcc-s1 lib32stdc++6 libasan5 libatomic1 libc-dev-bin libc6-dev libc6-i386 libclang-common-10-dev libclang-cpp10
  libclang1-10 libcrypt-dev libffi-dev libgc1c2 libgcc-9-dev libgomp1 libitm1 libllvm10 liblsan0 libncurses-dev libobjc-9-dev libobjc4 libomp-10-dev
  libomp5-10 libpfm4 libquadmath0 libstdc++-9-dev libtinfo-dev libtsan0 libubsan1 libz3-4 libz3-dev linux-libc-dev llvm-10 llvm-10-dev llvm-10-runtime
  llvm-10-tools manpages-dev python3-pygments
Suggested packages:
  clang-10-doc glibc-doc ncurses-doc libomp-10-doc libstdc++-9-doc llvm-10-doc python-pygments-doc ttf-bitstream-vera
The following NEW packages will be installed:
  binfmt-support clang clang-10 gcc-9-base lib32gcc-s1 lib32stdc++6 libasan5 libatomic1 libc-dev-bin libc6-dev libc6-i386 libclang-common-10-dev
  libclang-cpp10 libclang1-10 libcrypt-dev libffi-dev libgc1c2 libgcc-9-dev libgomp1 libitm1 libllvm10 liblsan0 libncurses-dev libobjc-9-dev libobjc4
  libomp-10-dev libomp5-10 libpfm4 libquadmath0 libstdc++-9-dev libtinfo-dev libtsan0 libubsan1 libz3-4 libz3-dev linux-libc-dev llvm-10 llvm-10-dev
  llvm-10-runtime llvm-10-tools manpages-dev python3-pygments
  
  $ apt show clang
  Clang project is a C, C++, Objective C and Objective C++ front-end
 for the LLVM compiler. Its goal is to offer a replacement to the GNU Compiler
 Collection (GCC).

```

GCC vs Clang:

> https://www.incredibuild.com/blog/gcc-vs-clang-battle-of-the-behemoths

