# libc Stdout Buffering

> [https://eklitzke.org/stdout-buffering](https://eklitzke.org/stdout-buffering)

Most programming languages offered buffered I/O features by default, since it makes generating output much more efficient. These buffered I/O facilities typically "Just Work" out of the box. But sometimes they don't. When we say they "don't work" what we mean is that excess buffering occurs, causing data not to be printed in a timely manner. This is typically fixed by explicitly putting a "flush" call in the code, e.g. with something like [sys.stdout.flush()](https://docs.python.org/2.6/library/sys.html#sys.stdin) in Python, [fflush(3)](http://man7.org/linux/man-pages/man3/fflush.3.html) in C, or [std::flush](http://en.cppreference.com/w/cpp/io/manip/flush) in C++.

Frequently when people are confused about the rules of buffering their code becomes littered with unnecessary flush statements, an example of [cargo-cult programming](http://www.catb.org/jargon/html/C/cargo-cult-programming.html). In this post I'll explain the buffering rules for stdout, so you'll never be confused again.

## Why Buffering Exists

As already discussed, the problem with buffering is that it can cause output to be delayed. So why does it exist at all?

At the underlying system call level, data is written to file descriptors using [write(2)](http://man7.org/linux/man-pages/man2/write.2.html). This method takes a file descriptor and a byte buffer, and writes the data in the byte buffer to the file descriptor.

Most languages have very fast function calls. The overhead for a function call in a compiled language like C or C++ is just a few CPU cycles. In these languages it's common to think of functions call overhead as negligible, and only in extreme cases are functions marked as [inline](https://en.wikipedia.org/wiki/Inline_function). However, a system call is much more expensive. A system call on Linux takes closer to a thousand CPU cycles and implies a [context switch](https://en.wikipedia.org/wiki/Context_switch). Thus system calls are significantly more expensive than regular userspace function calls.

The main reason why buffering exists is to amortize the cost of these system calls. This is primarily important when the program is doing a **lot** of these write calls, as the amortization is only effective when the system call overhead is a significant percentage of the program's time.

Let's consider what happens when you use [grep](http://man7.org/linux/man-pages/man1/grep.1.html) to search for a pattern in an input file (or stdin). Suppose you're grepping nginx logs for a pattern---say lines from a particular IP address. A typical line length in these nginx logs might be 100 characters. That means that if buffering wasn't used, for each matching line in the input file that grep needs to print, it would invoke the `write(2)` system call. This would happen over and over again, and each time the average buffer size would be 100 bytes. If, instead, a 4096-byte buffer size is used then data won't be flushed until the 4096-byte buffer fills up. This means that in this mode the grep command would wait until it had about 40 lines of input before the byte buffer filled up. Then it would flush the buffer by invoking `write(2)` with a pointer to the 4096-byte buffer. This effectively transforms forty system calls into one, yielding a 40x decrease in system call overhead. Not bad!

If the grep command is sending a lot of data to stdout you won't even notice the buffering delay. And a grep command matching a simple pattern can easily spend more time trying to print data than actually filtering the input data. But suppose instead the grep pattern occurs very infrequently. Suppose it's so uncommon that a matching input line is only found once every 10 seconds. Then we'd have to wait about 400 seconds (more than six minutes!) before seeing *any* output, even though grep actually found data within the first ten seconds.

## Buffering Anti-Patterns

This buffering can be especially insidious in certain shell pipelines. For instance, suppose we want to print the first matching line in a log file. The invocation might be:

```bash
# BAD: grep will buffer output before sending it to head
grep RAREPATTERN /var/log/mylog.txt | head -n 1
```

Going with the previous example, we would like this command to complete within ten seconds, since that's the average amount of time it will take grep to find the input pattern in this file. But if buffering is enabled then the pipeline will instead take many minutes to run. In other words, in this example buffering makes the program strictly slower, not faster!

Even in cases where the output isn't being limited by a command like `head(1)`, if output is very infrequent then buffering can be extremely annoying and provide essentially zero performance improvement.

## When Programs Buffer, And When They Don't

There are typically three modes for buffering:

- If a file descriptor is **unbuffered** then no buffering occurs whatsoever, and function calls that read or write data occur immediately (and will block).
- If a file descriptor is **fully-buffered** then a fixed-size buffer is used, and read or write calls simply read or write from the buffer. The buffer isn't flushed until it fills up.
- If a file descriptor is **line-buffered** then the buffering waits until it sees a newline character. So data will buffer and buffer until a `\n` is seen, and then all of the data that buffered is flushed at that point in time. In reality there's typically a maximum size on the buffer (just as in the fully-buffered case), so the rule is actually more like "buffer until a newline character is seen or 4096 bytes of data are encountered, whichever occurs first".

GNU libc (glibc) uses the following rules for buffering:

| Stream             | Type   | Behavior       |
| ------------------ | ------ | -------------- |
| stdin              | input  | line-buffered  |
| stdout (TTY)       | output | line-buffered  |
| stdout (not a TTY) | output | fully-buffered |
| stderr             | output | unbuffered     |

As you can see, the behavior for stdout is a bit unusual: the exact behavior for stdout depends on whether or not [it appears to be a TTY](http://man7.org/linux/man-pages/man3/isatty.3.html). The rationale here is that when stdout is a TTY it means a user is likely watching the command run and waiting for output, and therefore printing data in a timely manner is most important. On the other hand, if the output isn't a TTY the assumption is that the data is being processed or saved for later use, and therefore efficiency is more important.

Most other programming languages have exactly the same rules: either because those languages implement function routines as calls to buffered libc output commands (such as [printf(3)](http://man7.org/linux/man-pages/man3/printf.3.html)), or because they actually implement the same logic.

## More Grep Examples

Grep is a special case for buffering because a grep command can turn a large amount of input data into a slow and small stream of output data. Therefore grep is particularly susceptible to buffering frustration. Knowing when grep will buffer data is easy: it follows the glibc buffering rules described above.

**If the output of grep is a TTY then it will be line-buffered. If the output of grep is sent to a file or a pipe, it will be fully-buffered, as the output destination is not a TTY.**

### Examples

This grep command will be line-buffered, since stdout is a TTY:

```bash
# line-buffered
grep RAREPATTERN /var/log/mylog.txt
```

If stdout is redirected to a file then stdout is no longer a TTY, and output will be fully-buffered. This is usually fine:

```bash
# fully-buffered
grep RAREPATTERN /var/log/mylog.txt >output.txt
```

One situation where the previous example isn't ideal is if you have another terminal output that is trying to `tail -f` the output file.

Suppose we want to search the file backwards by piping [tac(1)](http://man7.org/linux/man-pages/man1/tac.1.html) to grep. This will be line-buffered, as grep is still the last command in the pipeline and thus stdout is still a TTY:

```bash
# line-buffered
tac /var/log/mylog.txt | grep RAREPATTERN
```

But what if we want to filter the output of grep? If we use a shell pipeline this will cause the grep output to become buffered. For instance, consider the following:

```bash
# fully-buffered
grep RAREPATTERN /var/log/mylog.txt | cut -f1
```

The issue here is that when we put a pipe after the grep command, grep's stdout is now the file descriptor for a pipe. **Pipes are not TTYs**, and thus grep will go into fully-buffered mode.

For the grep command the solution is to use the `--line-buffered` option to force line-buffering:

```bash
# forced line-buffering
grep --line-buffered RAREPATTERN /var/log/mylog.txt | cut -f1
```

As noted earlier, you may also want to use this when redirecting grep output to a file and then consuming the file in another session using `tail -f`.



## Setbuf

If you're writing your own C code, you can control the buffering for `FILE*` streams using [setbuf(3)](http://man7.org/linux/man-pages/man3/setbuf.3.html). Using this you can force behavior such as always line-buffering stdout. You can also use this for disk-backed files, so you can do things like write a file to disk and have [fprintf(3)](http://man7.org/linux/man-pages/man3/printf.3.html) be automatically line-buffered.

## Stdbuf

GNU coreutils comes with a program called [stdbuf](https://www.gnu.org/software/coreutils/manual/html_node/stdbuf-invocation.html) that allows you to change the default buffering behavior of programs you don't control. There are a few caveats for target programs: the programs must use C `FILE*` streams, and the programs can't use the explicit buffer control routines like `setbuf(3)`.

## C++ IOStreams

There's one further gotcha that typically pops up in C++ programs. Many C++ programmers are accustomed to using [std::endl](http://en.cppreference.com/w/cpp/io/manip/endl) for newlines. For instance,

```c++
// Two ways to print output with a newline ending.
std::cout << "Hello, world!\n";
std::cout << "Hello, world!" << std::endl;
```

These are **not the same**. The difference is that when `std::endl` is used it automatically forces the output stream to be flushed, regardless of the output mode of the stream. For instance,

```c++
// Subject to normal buffering rules.
std::cout << "Hello, world!\n";

// These are equivalent and are *always* line-buffered.
std::cout << "Hello, world!\n" << std::flush;
std::cout << "Hello, world!" << std::endl;
```

Thus if you're using `std::endl` a lot then the usual buffering rules don't apply, since `std::endl` is effectively forcing line-buffering! This can be important in certain performance sensitive programs, since using `std::endl` can inadvertently disable buffering.

My suggestion is: only use `std::endl` when you actually want to flush the output stream. If you don't know if the stream should be forcibly flushed then stick to using a regular `\n` sequence in your code.
