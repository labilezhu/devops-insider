# loading lib with libc - dlopen



## Loading, unloading & reloading shared libraries

Using dynamic linker functions to load, unload and reload our code into a process.



> source: [https://aixxe.net/2016/09/shared-library-injection](https://aixxe.net/2016/09/shared-library-injection)



Recently I've been working on various projects for different Source games, all of which are loaded into the game process as a shared library. Everything works well, apart from when any changes need to be made to the code. Up until now I've been restarting the game every time I changed even a single line, simply because I wasn't sure how to unload the library. Fortunately, this is no longer an issue.

Firstly, let's think about how we might load our library into a target process. There are a few methods and tools I've tried, including [linux-inject](https://github.com/gaffe23/linux-inject) - which I couldn't get to work at all, and [hotpatch](https://github.com/vikasnkumar/hotpatch), which worked well but only for 64-bit processes. Putting our library in the `LD_PRELOAD` variable is another notable mention but we would rather load our code on-demand. That said, I've had a 100% success rate with both 32-bit and 64-bit processes using the GNU Debugger, or gdb for short.

### Loading the library

Make sure that you have a constructor and destructor function in your library. As you might expect, the constructor function will be called when the library is loaded and the destructor will be called before it is unloaded from the process.

```cpp
void __attribute__((constructor)) startup();
void __attribute__((destructor)) shutdown();
```

Run gdb with elevated permissions and attach to the target process. Once it's finished loading you'll be left with a command shell. Depending on your distribution, you might need to [allow ptrace](https://wiki.ubuntu.com/SecurityTeam/Roadmap/KernelHardening#ptrace_Protection) to be used on non-child processes.

```
$ sudo gdb -ex "attach $(pidof csgo_linux64)"
```

Export the functions we're going to use into some variables to make things easier for ourselves. If you're wondering where the arguments came from, they're the function prototypes for dlopen and dlclose from the glibc [dlfcn header](https://github.com/bminor/glibc/blob/master/include/dlfcn.h#L102-L103).

```
(gdb) set $dlopen = (void*(*)(char*, int)) dlopen
(gdb) set $dlclose = (int(*)(void*)) dlclose
```

Now we can finally load our library. To do this, we'll use `dlopen` which will automatically invoke the constructor function. This will return a handle which we can use later when we've finished testing and need to unload our library.

```
(gdb) set $library = $dlopen("/home/aixxe/devel/rekoda-csgo/librekoda-csgo.so", 1)
```

It probably looks like nothing happened but if everything went well then our library is now loaded.



We're all done for now so we can safely resume the target process and test our code.

```
(gdb) continue
Continuing.
```

### Unloading the library

Now that we've finished testing it turns out we need to change something, we're going to recompile our library and load it again. This is usually where I would restart the process but this is no longer necessary. Switch back to gdb and press CTRL+C to send an interrupt signal. This will suspend the process again and we should be back in the debugging shell.

```
Thread 1 "csgo_linux64" received signal SIGINT, Interrupt.
(gdb) call $dlclose($library)
(gdb) continue
Continuing.
```



Now that the reference counter for our library has dropped to zero it should automatically be unloaded.

### Reloading the library

We've made our changes to the code and recompiled. Let's load it back into the game process now. We already have our gdb session from earlier running so we only need to call `dlopen` again.

```
(gdb) set $library = $dlopen("/home/aixxe/devel/rekoda-csgo/librekoda-csgo.so", 1)
```



It's that simple. Actually, this whole thing is a bit tedious in the long run, we should write some scripts to automate this as soon as possible. Here's one I prepared earlier, this is just for loading.

```
if grep -q \"$(realpath $2)\" /proc/$(pidof $1)/maps; then
  exit
fi

gdb -n -q -batch -ex "attach $(pidof $1)" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "call \$dlopen(\"$(realpath $2)\", 1)" \
  -ex "detach" \
  -ex "quit"
```

And this one is for unloading. Both take the same arguments, the process name and the library filename.

```
if grep -q \"$(realpath $2)\" /proc/$(pidof $1)/maps; then
  gdb -n -q -batch -ex "attach $(pidof $1)" \
    -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
    -ex "set \$dlclose = (int(*)(void*)) dlclose" \
    -ex "set \$library = \$dlopen(\"$(realpath $2)\", 6)" \
    -ex "call \$dlclose(\$library)" \
    -ex "call \$dlclose(\$library)" \
    -ex "detach" \
    -ex "quit"
fi
```

Note that `dlclose` is called twice in the unloading script. Since these are two separate scripts we can't keep the handle we get from loading and use it in the unloading script. So when we call `dlopen` again in the unloading script we increment the reference count to two.

> The dl library maintains reference counts for library handles, so a dynamic library is not deallocated until dlclose() has been called on it as many times as dlopen() has succeeded on it.
>
> — dlopen(3)

Therefore, we have to call `dlclose` twice in order to decrement the reference count to zero. That's all! You can even add the scripts to your Makefile if you want to automatically reload when recompiling.

You may also want to add `-static-libstdc++` to your linker options if you're having issues with some features of the standard template library, or if the destructor is called as soon as you load your library.





```
TARGET = csgo_linux64
OUT := librekoda-csgo.so
detach:
  sudo ./detach.sh $(TARGET) $(OUT)

attach:
  sudo ./attach.sh $(TARGET) $(OUT)
```



