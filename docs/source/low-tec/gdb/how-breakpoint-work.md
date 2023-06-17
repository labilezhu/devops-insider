
> https://mohit.io/blog/software-breakpoints/

# Software Breakpoints

_Line breakpoints can be implemented either in hardware or software.  This article discusses the latter in detail._

It is very useful to be able to break execution of code at a line number of your choice.  Breakpoints are provided in debuggers to do exactly that.  It is fun getting to the root of the problem by setting breakpoints in a debug session.  It is even more fun to know how do breakpoints work in the first place.

Software breakpoints work by  inserting a special instruction in the program being debugged.  This special instruction on the Intel platform is “int 3”.  When executed it calls the debugger’s exception handler.

### Example

Let us look at a very simple example that inserts a breakpoint in a program at compile time and not through a debugger.  The code uses the Intel instruction “int 3” and you may need to figure out the equivalent instruction for a non-Intel platform.  

```c
// The code below works well with Visual Studio.
int main()
{
    __asm int 3;
    printf("Hello World\n");
    return 0;
}
 
// The code below works well with gcc + gdb
int main()
{
    asm("int $3");
    printf("Hello World\n");
    return 0;
}
```


If you run this program in Visual Studio, you get a dialog saying “_**helloworld****.exe has triggered a breakpoint**_“.

In gdb you get the message “**_Program received signal SIGTRAP, Trace/breakpoint trap._**”

In the example above, a call to “int 3” invokes the debugger’s exception handler.

It is also interesting to note the assembly instructions generated for the program above.

In Visual Studio, right click on the code and click on “Show Disassembly”. Also ensure that “Show Code Bytes” is on in the same context menu.

![4418127ed49a0d38d5a8d060ca4faeee.png](assets/4418127ed49a0d38d5a8d060ca4faeee.png)


In gdb type disassemble at the gdb command.

> disassemble
> ```asm
> 0x0040107a <main+42>:   int3
> ```


Now obtain the opcode of the int3 instruction using the x (examine memory) command

> (gdb) x/x main+42
> ```asm
> 0x40107a <main+42>:     0xcc
> ```

As seen above, the breakpoint opcode we inserted during compilation is 0xCC .

### How Do Debuggers Insert Breakpoints?

For practical reasons, it is unwise to ask for a recompilation whenever a breakpoint is added or deleted.  Debuggers change the loaded image of the executable in memory and insert the “int 3” instruction at runtime.  The common steps a debugger performs to provide the functionality of a line breakpoint to a user are as follows –

1. When a user inserts a breakpoint in a line of code, the debugger saves the opcode at that given location and replaces it with 0xCC (int 3).
2. When the program is run and it executes the “int 3” instruction, control is passed to the debugger’s exception handler.
3. The debugger notifies the user that a breakpoint has been hit. Say that the user instructs the debugger to resume execution of the program.
4. The debugger replaces the opcode 0xCC with the one it had saved earlier.  This is done to restore the instructions to their original state.
5. The debugger then single steps the program.
6. It then resaves the original instruction and re-inserts the opcode 0xCC.  If this step were not done, the breakpoint would have been lost.  [Temporary breakpoints](https://mohit.io/blog/temporary-breakpoint-now-you-see-it-now-you-dont/) on the other hand skip this step.
7. The debugger then resumes execution of the program.

Hardware breakpoints are limited in number but debuggers are able to provide unlimited breakpoints by implementing them through software.

Knowing what goes behind the scenes makes debugging a bit easier.  A debugger may defer setting a breakpoint if the module is not loaded in memory yet.  It needs to replace some opcode with 0xCC and that can happen only when the module is in memory.  Likewise, a mismatch between a binary, its sources and its debug symbols (or the lack of it) may cause breakpoints to be hit at unexpected locations because the debugger is not able to correctly map the source line to the opcode that it needs to replace with 0xCC.  At times debuggers complain about the mismatch and refuse to set the breakpoints.

Many of the setup issues with breakpoints become obvious once we know how they work internally.  And when all else fails and release build breakpoints  adamantly refuse to work, you always have the option of compiling an “int 3” breakpoint right into your code.

> https://stackoverflow.com/questions/37857768/how-does-gdb-restore-instruction-after-breakpoint

On Linux, to continue past the breakpoint, `0xCC` is replaced with the original instruction, and `ptrace(PTRACE_SINGLESTEP, ...)` is done. When the thread stops on the next instruction, original code is again replaced by the `0xCC` opcode (to restore the breakpoint), and the thread continued on its merry way.

On x86 platforms that do not have `PTRACE_SINGLESTEP`, [trap flag](https://en.wikipedia.org/wiki/Trap_flag) is set in `EFLAGS` via `ptrace(PTRACE_SETREGS, ...)` and the thread is continued. The trap flag causes the thread to immediately stop again (on the next instruction, just like `PTRACE_SINGLESTEP` would).

> When i type disassemble in GDB, i do not see CC opcode. Is this because GDB knows it is him that puts the CC ?

Correct. A program could examine and print its own instruction stream, and you can observe breakpoint `0xCC` opcodes that way.

> Is there a way to do a raw disassemble, in order to see exactly what opcodes are loaded in memory at this instant ?

I don't believe there is. You can use `(gdb) set debug infrun` to observe what GDB is doing to the inferior (being debugged) process.



> https://eli.thegreenplace.net/2011/01/27/how-debuggers-work-part-2-breakpoints

