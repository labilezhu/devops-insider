# inject sharelib into process

## hotpatch

- [https://github.com/vikasnkumar/hotpatch](https://github.com/vikasnkumar/hotpatch)


### Introduction to Hotpatch

Hotpatch is a library that can be used to dynamically load a shared library
(.so) file on Linux from one process into another already running process,
without affecting the execution of the target process. The API is a C API, but
also supported in C++.

The current version is 0.2.

The limitations, directions on how to use, and possible uses of hotpatch will be
explained in this document.

The main idea of hotpatch stems from the fact that in Linux, it is not easy to
load a library into another already running process. In Windows, there is an API
called CreateRemoteThread() that can load a library into another process very
easily with a couple of API calls. Hotpatch makes this functionality available
to Linux users and developers, with a single API call. Unlike other available
injection libraries, hotpatch restores the execution of the process to its
original state.

The user can do the following with hotpatch:
- load his/her own .so file into an already running process
- invoke a custom symbol/function in that .so file
- pass arguments to that function as long as it is serialized to the form of a
  byte buffer and length of the buffer. This shall be explained more later.



## with libc
```{toctree}
loading-lib-with-libc-dlopen.md
```